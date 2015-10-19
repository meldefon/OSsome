// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>

using namespace std;

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

int Acquire_Syscall(int id) { 
  //sysLock.Acquire();
  if(id > (locks.size() - 1) || id < 0) { 
    //sysLock.Release();
    return -1; //if the id they gave us is bad, return -1
  } else { //they gave us a valid id, lets check if it's in the same address space
    KernelLock *kl = locks[id]; //grab the struct

    if(kl->addrSpace != currentThread->space) {
      return -1; //if not the same address space, return -1
    }

    kl->lock->Acquire(); //acquire the lock
    return 0; //return 0 once acquired
  }
}

int Release_Syscall(int id) { 
  //sysLock.Acquire();
  if(id > (locks.size() - 1) || id < 0) { 
    //sysLock.Release();
    return -1; //if the id they gave us is bad, return -1
  } else { //they gave us a valid id, lets check if it's in the same address space
    KernelLock *kl = locks[id]; //grab the struct

    if(kl->addrSpace != currentThread->space) {
      return -1; //if not the same address space, return -1
    }

    kl->lock->Release(); //release the lock
    return 0; //return 0 once released
  }
}

int Wait_Syscall(int c, int l) { 
  //sysLock.Acquire();
  //sysCondition.Acquire();
  if((l > (locks.size() - 1) || l < 0) || (c > (conditions.size() - 1) || c < 0)) { 
    //sysLock.Release();
    //sysCondition.Release();
    return -1; //if the id they gave us is bad, return -1
  } else { //they gave us a valid id, lets check if it's in the same address space
    KernelLock *kl = locks[l]; //grab the struct
    KernelCondition *kc = conditions[c]; //grab the struct

    if((kl->addrSpace != currentThread->space) || (kc->addrSpace != currentThread->space)) {
      return -1; //if not the same address space, return -1
    }

    kc->condition->Wait(kl->lock); //wait
    return 0; //we were able to wait
  } 
}

int Signal_Syscall(int c, int l) { 
  //sysLock.Acquire();
  //sysCondition.Acquire();  
  if((l > (locks.size() - 1) || l < 0) || (c > (conditions.size() - 1) || c < 0)) { 
    //sysLock.Release();
    //sysCondition.Release();    
    return -1; //if the id they gave us is bad, return -1
  } else { //they gave us a valid id, lets check if it's in the same address space
    KernelLock *kl = locks[l]; //grab the struct
    KernelCondition *kc = conditions[c]; //grab the struct

    if((kl->addrSpace != currentThread->space) || (kc->addrSpace != currentThread->space)) {
      return -1; //if not the same address space, return -1
    }

    kc->condition->Signal(kl->lock); //signal
    return 0; //we were able to signal
  } 
}

int Broadcast_Syscall(int c, int l) {
  //sysLock.Acquire();
  //sysCondition.Acquire();
  if((l > (locks.size() - 1) || l < 0) || (c > (conditions.size() - 1) || c < 0)) { 
    //sysLock.Release();
    //sysCondition.Release();
    return -1; //if the id they gave us is bad, return -1
  } else { //they gave us a valid id, lets check if it's in the same address space
    KernelLock *kl = locks[l]; //grab the struct
    KernelCondition *kc = conditions[c]; //grab the struct

    if((kl->addrSpace != currentThread->space) || (kc->addrSpace != currentThread->space)) {
      return -1; //if not the same address space, return -1
    }

    kc->condition->Broadcast(kl->lock); //broadcast
    return 0; //we were able to broadcast
  }
}

int CreateLock_Syscall() {
  KernelLock *kl = new KernelLock(); //create new struct
  Lock *l = new Lock(); //create new lock
  
  kl->lock = l; //assign lock pointer 
  kl->addrSpace = currentThread->space; //assign address space

  //sysLock.Acquire();
  locks.push_back(kl); //add new struct to our locks vector
  int index = locks.size() - 1;
  //sysLock.Release();
  return index;; //return new index of lock
}

int DestroyLock_Syscall(int id) { 
  //sysLock.Acquire();
  if(id > (locks.size() - 1) || id < 0) { 
    //sysLock.Release();
    return -1; //if the id they gave us is bad, return -1
  } else { //they gave us a valid id, lets check if it's in the same address space
    KernelLock *kl = locks[id]; //grab the struct

    if(kl->addrSpace != currentThread->space) {
      return -1; //if not the same address space, return -1
    }

    if(kl->isToBeDeleted == true){
      return -1;
    }

    kl->isToBeDeleted = true; //set it to be deleted

    return 0; //return 0
  } 
}

int CreateCondition_Syscall() {
  KernelCondition *kc = new KernelCondition(); //create new struct
  Condition *c = new Condition(); //create new condition
  
  kc->condition = c; //assign condition pointer 
  kc->addrSpace = currentThread->space; //assign address space

  //sysCondition.Acquire();
  conditions.push_back(kc); //add new struct to our conditions vector
  int index = conditions.size() - 1;
  //sysCondition.Release();
  return index; //return new index of condition 
}

int DestroyCondition_Syscall(int id) { 
  //sysCondition.Acquire();
  if(id > (conditions.size() - 1) || id < 0) { 
    //sysCondition.Release();
    return -1; //if the id they gave us is bad, return -1
  } else { //they gave us a valid id, lets check if it's in the same address space
    KernelCondition *kc = conditions[id]; //grab the struct

    if(kc->addrSpace != currentThread->space) {
      return -1; //if not the same address space, return -1
    }

    if(kc->isToBeDeleted == true)
      return -1;

    kc->isToBeDeleted = true; //set it to be deleted
    return 0; //return 0
  }
}

int Rand_syscall(int range, int offset) {
  int value = (rand() % range) + offset;
  return value;
}

int Scanf_syscall() {
  int num;
  scanf("%d", &num);
  return num;
}

void Printf_syscall(unsigned int vaddr, int length, int Num_1, int Num_2) {
    
    char* string;
    string = new char[length];
    copyin(vaddr,length,string);

    int lastIndex;
    int check = 0;

    int num_1 = Num_1 / 100000;
    int num_2 = Num_1 % 100000;
    int num_3 = Num_2 / 100000;
    int num_4 = Num_2 % 100000;

    for(int i = 0; i < length; i++) {
        if(string[i] == '%') {
            lastIndex = i;
        } else if(string[i] == 'd' && lastIndex == i - 1) {
            check++;
        }
    }

    if(check == 0) {
        printf(string);
    } else if(check == 1) {
        printf(string, num_1);
    } else if(check == 2) {
        printf(string, num_1, num_2);
    } else if(check == 3) {
        printf(string, num_1, num_2, num_3);
    } else if(check == 4) {
        printf(string, num_1, num_2, num_3, num_4);
    }
}

void Exit_Syscall(int status) {
    currentThread->Finish(); //Stop running the current thread

    //Simplest case - if nothing is left, just halt the whole run
    interrupt->Halt();
}

void kernel_exec(int unused){


    currentThread->space->InitRegisters();		// set the initial register values
    currentThread->space->RestoreState();		// load page table

    int nextStackAddr = currentThread->space->getNextStackAddr(); //Get next stack address
    machine->WriteRegister(StackReg, nextStackAddr); //Set next stack address
    DEBUG('e',"Setting first thread's stack address to %d\n",nextStackAddr);


    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"
}

SpaceId Exec_Syscall(int fileID) {

    //Get executable from open list, initialize address space
    //TODO This will break if the user is stupid, so we need to check if the ID is valid
    OpenFile *executable = (OpenFile *) currentThread->space->fileTable.Get(fileID);
    AddrSpace *space;

    if (executable == NULL) {
        printf("Unable to open file %d\n", fileID);
        return 0; //TODO fix
    }

    space = new AddrSpace(executable);
    delete executable;			// close file

    //Make new thread, set its new address space
    Thread* t = new Thread("");
    t->space = space;

    //After this point, we have no more control of the new process from here
    t->Fork(kernel_exec,0);

    return 0; //TODO fix

}

void kernel_thread(int vaddr){
    //DEBUG('a', "In kernel_thread about to run new thread\n");
    //Need to set the registers
    currentThread->space->InitRegisters();
    machine->WriteRegister(PCReg, vaddr);
    machine->WriteRegister(NextPCReg, vaddr + 4);
    int nextStackAddr = currentThread->space->getNextStackAddr();
    machine->WriteRegister(StackReg, nextStackAddr);
    DEBUG('f',"Assigning new thread's stack address to %d \n",nextStackAddr);
    //machine->WriteRegister(StackReg, numPages * PageSize - 16);
    machine->Run();
}

void Fork_Syscall(int forkArg) {
    //DEBUG('a',"Fork syscall being executed\n");
    Thread* t = new Thread("");
    t->space = currentThread->space;
    t->space->numThreads = t->space->numThreads+1;
    //int forkArg = machine->ReadRegister(4);
    t->Fork(kernel_thread,forkArg);
}


void Yield_Syscall() {
  currentThread->Yield();
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall
    DEBUG('a',"Syscall: %d \n",type);

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;         
      case SC_Exit:
    DEBUG('a', "Exit syscall.\n");
    Exit_Syscall(machine->ReadRegister(4));
    break;
      case SC_Exec:
    DEBUG('a', "Exec syscall.\n");
    Exec_Syscall(machine->ReadRegister(4));
    break;

    case SC_Fork:
    DEBUG('a', "Fork syscall.\n");
    Fork_Syscall(machine->ReadRegister(4));
    break;

    case SC_Yield:
    DEBUG('a', "Yield syscall.\n");
    Yield_Syscall();
    break;
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
 
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;
      case SC_Acquire:
    DEBUG('a', "Acquire syscall.\n");
    rv = Acquire_Syscall(machine->ReadRegister(4));
    break;
      case SC_Release:
    DEBUG('a', "Release syscall.\n");
    rv = Release_Syscall(machine->ReadRegister(4));
    break;
      case SC_Wait:
    DEBUG('a', "Wait syscall.\n");
    rv = Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    break;
      case SC_Signal:
    DEBUG('a', "Signal syscall.\n");
    rv = Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    break;
      case SC_Broadcast:
    DEBUG('a', "Broadcast syscall.\n");
    rv = Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    break;
      case SC_CreateLock:
    DEBUG('a', "CreateLock syscall.\n");
    rv = CreateLock_Syscall();
    break;
      case SC_DestroyLock:
    DEBUG('a', "DestroyLock syscall.\n");
    rv = DestroyLock_Syscall(machine->ReadRegister(4));
    break;
      case SC_CreateCondition:
    DEBUG('a', "CreateCondition syscall.\n");
    rv = CreateCondition_Syscall();
    break;
      case SC_DestroyCondition:
    DEBUG('a', "DestroyCondition syscall.\n");
    rv = DestroyCondition_Syscall(machine->ReadRegister(4));
    break;
      case SC_Rand:
    DEBUG('a', "Rand syscall.\n");
    rv = Rand_syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    break;
      case SC_Printf:
    DEBUG('a', "Printf syscall.\n");
    Printf_syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6), machine->ReadRegister(7));
    break;
      case SC_Scanf:
    DEBUG('a', "Scanf syscall.\n");
    rv = Scanf_syscall();
    break;  
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
