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
  if(id > (locks.size() - 1) || id < 0) { 
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
  if(id > (locks.size() - 1) || id < 0) { 
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
  if((l > (locks.size() - 1) || l < 0) || (c > (conditions.size() - 1) || c < 0)) { 
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
  if((l > (locks.size() - 1) || l < 0) || (c > (conditions.size() - 1) || c < 0)) { 
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
  if((l > (locks.size() - 1) || l < 0) || (c > (conditions.size() - 1) || c < 0)) { 
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

  locks.push_back(kl); //add new struct to our locks vector
  cout<<"Exception: "<<"New Lock index: "<<locks.size() - 1<<"\n";
  return (locks.size() - 1); //return new index of lock
}

int DestroyLock_Syscall(int id) { 
  cout<<"Exception: "<<"Input id: "<<id<<"\n";
  if(id > (locks.size() - 1) || id < 0) { 
    cout<<"Exception: "<<id<<" is an invalid id\n";
    return -1; //if the id they gave us is bad, return -1
  } else { //they gave us a valid id, lets check if it's in the same address space
    KernelLock *kl = locks[id]; //grab the struct

    if(kl->addrSpace != currentThread->space) {
      cout<<"Exception: "<<id<<" is not in the same address space\n";
      return -1; //if not the same address space, return -1
    }

    if(kl->isToBeDeleted == true){
      cout<<"Exception: "<<"Lock "<<id<<" is already deleted\n";
      return -1;
    }

    kl->isToBeDeleted = true; //set it to be deleted
     cout<<"Exception: "<<"Deleted Lock "<<id<<"\n";

    return 0; //return 0
  } 
}

int CreateCondition_Syscall() {
  KernelCondition *kc = new KernelCondition(); //create new struct
  Condition *c = new Condition(); //create new condition
  
  kc->condition = c; //assign condition pointer 
  kc->addrSpace = currentThread->space; //assign address space

  conditions.push_back(kc); //add new struct to our conditions vector
  return (conditions.size() - 1); //return new index of condition 
}

int DestroyCondition_Syscall(int id) { 
  if(id > (conditions.size() - 1) || id < 0) { 
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

void Exit_Syscall(int status) {

    progLock->Acquire();
    int myPID = currentThread->space->processID;
    int numActiveThreads = processTable->at(myPID)->numThreads;

    int numActiveProcesses = 0;
    for(int i = 0;i<processTable->size();i++){
        if(processTable->at(i)->running){
            numActiveProcesses+=1;
        }
    }
    DEBUG('X',"Exit called on process %d with %d processes and %d threads\n",myPID,numActiveProcesses,numActiveThreads);


    //If you're the last thread of the last process, kill the machine
    if(numActiveThreads==1 && numActiveProcesses==1){
        DEBUG('X',"Last thread. Killing execution\n");
        interrupt->Halt();
        ASSERT(FALSE);
        return;
    }


    //If you're the last thread in your nonlast process, clear your memory, set your process to not
    //running, and delete your address space
    else if(numActiveThreads==1){
        DEBUG('X',"Process's last thread, deallocating address space\n");
        //Clear all your pages
        for(int i = 0;i<currentThread->space->numPages;i++){
            freePageBitMap->Clear(currentThread->space->pageTable[i].physicalPage);
        }

        //Set your process to not running
        processTable->at(currentThread->space->processID)->running = false;

        //Delete your addressspace
        delete currentThread->space;

        //return;
    }

    //If you're the nonlast thread in your process, just clear your stack pages
    else {
        DEBUG('X',"Nonlast thread, freeing up stack memory\n");
        //compute stack page numbers
        int endingPage = divRoundUp(currentThread->baseStackAddr + 16, PageSize);
        int numStackPages = divRoundUp(UserStackSize, PageSize);
        int numCodeDataPages = currentThread->space->numNonStackPages;
        DEBUG('X',"Freeing stack pages %d through %d\n",endingPage-numStackPages,endingPage);
        for (int i = endingPage - numStackPages + 1; i <= endingPage; i++) {

            //Don't want to actually clear physical memory
            //freePageBitMap->Clear(currentThread->space->pageTable[i].physicalPage);
            currentThread->space->stackBitMap.Clear(i - numCodeDataPages);
        }
        //Decrement the number of threads
        processTable->at(currentThread->space->processID)->numThreads -= 1;
    }



    progLock->Release();
    currentThread->Finish(); //Stop running the current thread

    //Simplest case - if nothing is left, just halt the whole run
    interrupt->Halt();
}

void kernel_exec(int unused){


    currentThread->space->InitRegisters();		// set the initial register values
    currentThread->space->RestoreState();		// load page table

    int nextStackAddr = currentThread->space->getNextStackAddr(); //Get next stack address
    currentThread->baseStackAddr = nextStackAddr;
    machine->WriteRegister(StackReg, nextStackAddr); //Set next stack address
    DEBUG('f',"Setting first thread's stack address to %d\n",nextStackAddr);


    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"
}

SpaceId Exec_Syscall(unsigned int vaddr,int len) {

    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return 0;

    if( copyin(vaddr,len,buf) == -1 ) {
        printf("%s","Bad pointer passed to Create\n");
        delete buf;
        return 0;
    }

    buf[len]='\0';

    progLock->Acquire();
    //Get executable from open list, initialize address space
    //TODO This will break if the user is stupid, so we need to check if the ID is valid
    //OpenFile *executable = (OpenFile *) currentThread->space->fileTable.Get(fileID);
    OpenFile *executable = fileSystem->Open(buf);
    AddrSpace *space;

    if (executable == NULL) {
        printf("Unable to open file %s\n", buf);
        return 0; //TODO fix
    }

    space = new AddrSpace(executable);
    //delete executable;			// close file

    //Make new process table entry, add to processTable
    ProcessStruct* processEntry  = new ProcessStruct();
    processEntry->pID = processTable->size()+1;
    processEntry->numThreads = 1;
    processEntry->running = true;
    processTable->push_back(processEntry);
    DEBUG('X',"Process table now has %d entries (not necessarily all active)\n",processTable->size());

    //Make new thread, set its new address space
    Thread* t = new Thread("Execed");
    t->space = space;

    progLock->Release();

    //After this point, we have no more control of the new process from here
    t->Fork(kernel_exec,0);

    return 0; //TODO fix

}

void kernel_thread(int vaddr){
    //DEBUG('a', "In kernel_thread about to run new thread\n");
    //Need to set the registers
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();		// load page table
    machine->WriteRegister(PCReg, vaddr);
    machine->WriteRegister(NextPCReg, vaddr + 4);
    int nextStackAddr = currentThread->space->getNextStackAddr();
    //Store the base stack address for deallocation on exti
    currentThread->baseStackAddr = nextStackAddr;
    machine->WriteRegister(StackReg, nextStackAddr);
    DEBUG('f',"Assigning new thread's stack address to %d \n",nextStackAddr);
    //machine->WriteRegister(StackReg, numPages * PageSize - 16);
    machine->Run();
}


void Fork_Syscall(int forkArg) {
    DEBUG('s',"Fork syscall being executed\n");
    Thread* t = new Thread("Forked");
    t->space = currentThread->space;

    //t->space->numThreads = t->space->numThreads+1;
    //int forkArg = machine->ReadRegister(4);
    processTable->at(t->space->processID)->numThreads = processTable->at(t->space->processID)->numThreads+1;
    DEBUG('X',"Process %d now has %d running threads\n",currentThread->space->processID,processTable->size());
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
    Exec_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
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
