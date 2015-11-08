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
#include <stdlib.h>
#include <iostream>
#include <sstream>

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

int convertMessageToInt(char* inBuffer) {
    //Declare our stream so we can pull out the int
    stringstream str;
    string word(inBuffer);
    int val;

    //Put the string into the stream then pull the int value out
    str << word;
    str >> val;

    return val;
}

void sendAndRecieveSyscallMessage(char* msg,char* inBuffer){

    //Declare headers
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;

    //Set up headers (Packet header does not need the from field set, since it will be set automatically on send).
    outPktHdr.to = SERVER_M;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->baseStackAddr; //TODO EVEN THIS IS NOT UNIQUE OVER THREADS, Since two threads could
    //TODO have the same base stack address if virtual memory is being used.
    outMailHdr.from = 0;
    outMailHdr.length = strlen(msg) + 1;

    //Send message
    DEBUG('N',msg);
    DEBUG('N',"\n");
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg);
    if ( !success ) {
        printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
        interrupt->Halt();
    }

    //Receive reply message
    postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, inBuffer); //For now, don't recieve because server isn't set up

    return;

}

int Acquire_Syscall(int id) {
    int val;

    //Making syscall message
    stringstream ss;
    ss<<SC_Acquire<<" "<<id;
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);

    //Convert the message to int
    val = convertMessageToInt(inBuffer);
    return val;

    /*
    //sysLock.Acquire();
    if (id > (locks.size() - 1) || id < 0) {
        val = -1; //if the id they gave us is bad, return -1
    } else { //they gave us a valid id, lets check if it's in the same address space
        KernelLock *kl = locks[id]; //grab the struct

        if (kl != NULL) {
            if (kl->addrSpace != currentThread->space) {
                val = -1; //if not the same address space, return -1
            } else {
                kl->lock->Acquire(); //acquire the lock
                val = 0; //return 0 once acquired
            }
        } else {
            val = -1;
        }
    }
    //sysLock.Release();
    */
}

int Release_Syscall(int id) {
    int val;
    //sysLock.Acquire();

    //Making syscall message
    stringstream ss;
    ss << SC_Release << " " << id;
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);

    //Convert the message to int
    val = convertMessageToInt(inBuffer);
    return val;

    /*
    if (id > (locks.size() - 1) || id < 0) {
        val = -1; //if the id they gave us is bad, return -1
    } else { //they gave us a valid id, lets check if it's in the same address space
        KernelLock *kl = locks[id]; //grab the struct

        if (kl != NULL) {
            if (kl->addrSpace != currentThread->space) {
                val = -1; //if not the same address space, return -1
            } else {
                kl->lock->Release(); //release the lock
                val = 0; //return 0 once released
            }
        } else {
            val = -1;
        }
    }
    //sysLock.Release();
    */
}

int Wait_Syscall(int c, int l) {
    int val;
    //sysLock.Acquire();
    //sysCondition.Acquire();

    //Making syscall message
    stringstream ss;
    ss << SC_Wait << " " << c << " " << l;
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);

    //Convert the message to int
    val = convertMessageToInt(inBuffer);

    //This means we were woken up and must go on to acquire
    if(val == -2) {
        val = Acquire_Syscall(l);
    }

    return val;

    /*
    if ((l > (locks.size() - 1) || l < 0) || (c > (conditions.size() - 1) || c < 0)) {
        val = -1; //if the id they gave us is bad, return -1
    } else { //they gave us a valid id, lets check if it's in the same address space
        KernelLock *kl = locks[l]; //grab the struct
        KernelCondition *kc = conditions[c]; //grab the struct

        if (kl != NULL && kc != NULL) {
            if ((kl->addrSpace != currentThread->space) || (kc->addrSpace != currentThread->space)) {
                val = -1; //if not the same address space, return -1
            } else {

                kc->condition->Wait(kl->lock); //wait
                val = 0; //we were able to wait
            }
        } else {
            val = -1;
        }
    }
    //sysLock.Release();
    //sysCondition.Release();
    */
}

int Signal_Syscall(int c, int l) {
    int val;
    //sysLock.Acquire();
    //sysCondition.Acquire();

    //Making syscall message
    stringstream ss;
    ss << SC_Signal << " " << c << " " << l;
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);
    
    //Convert the message to int
    val = convertMessageToInt(inBuffer);
    return val;

    /*
    if ((l > (locks.size() - 1) || l < 0) || (c > (conditions.size() - 1) || c < 0)) {
        val = -1; //if the id they gave us is bad, return -1
    } else { //they gave us a valid id, lets check if it's in the same address space
        KernelLock *kl = locks[l]; //grab the struct
        KernelCondition *kc = conditions[c]; //grab the struct

        if (kl != NULL && kc != NULL) {
            if ((kl->addrSpace != currentThread->space) || (kc->addrSpace != currentThread->space)) {
                val = -1; //if not the same address space, return -1
            } else {
                kc->condition->Signal(kl->lock); //signal
                val = 0; //we were able to signal
            }
        } else {
            val = -1;
        }
    }
    //sysLock.Release();
    //sysCondition.Release();
    */
}

int Broadcast_Syscall(int c, int l) {
    int val;
    //sysLock.Acquire();
    //sysCondition.Acquire();

    //Making syscall message
    stringstream ss;
    ss << SC_Broadcast << " " << c << " " << l;
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);

    //Convert the message to int
    val = convertMessageToInt(inBuffer);
    return val;

    /*
    if ((l > (locks.size() - 1) || l < 0) || (c > (conditions.size() - 1) || c < 0)) {
        val = -1; //if the id they gave us is bad, return -1
    } else { //they gave us a valid id, lets check if it's in the same address space
        KernelLock *kl = locks[l]; //grab the struct
        KernelCondition *kc = conditions[c]; //grab the struct

        if (kl != NULL && kc != NULL) {
            if ((kl->addrSpace != currentThread->space) || (kc->addrSpace != currentThread->space)) {
                val = -1; //if not the same address space, return -1
            } else {
                kc->condition->Broadcast(kl->lock); //broadcast
                val = 0; //we were able to broadcast
            }
        } else {
            val = -1;
        }
    }
    //sysLock.Release();
    //sysCondition.Release();
    */
}


int CreateLock_Syscall(unsigned int name, int len) {

    //Set up string lock name
    char *buf;        // Kernel buffer for output
    if (!(buf = new char[len])) {
        printf("%s", "Error allocating kernel buffer for write!\n");
        return -1;
    } else {
        if (copyin(name, len, buf) == -1) {
            printf("%s", "Bad name pointer\n");
            delete[] buf;
            return -1;
        }
    }
    buf[len] = '\0';


    //Making syscall message
    stringstream ss;
    ss << SC_CreateLock << " " << buf<<"@";
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char *) ss.str().c_str(), inBuffer);

    //Convert the message to int, which will contain either an error code or our new index
    int val = convertMessageToInt(inBuffer);
    return val;

    /*
    KernelLock *kl = new KernelLock(); //create new struct
    Lock *l = new Lock(); //create new lock

    kl->lock = l; //assign lock pointer
    kl->addrSpace = currentThread->space; //assign address space

    sysLock.Acquire();
    locks.push_back(kl); //add new struct to our locks vector
    int index = locks.size() - 1;
    sysLock.Release();
    return index;; //return new index of lock
    */
}

int DestroyLock_Syscall(int id) {
    int val;
    //sysLock.Acquire();

    //Making syscall message
    stringstream ss;
    ss << SC_DestroyLock << " " << id;
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);

    //Convert the message to int
    val = convertMessageToInt(inBuffer);
    return val;

    /*
    if (id > (locks.size() - 1) || id < 0) {
        val = -1; //if the id they gave us is bad, return -1
    } else { //they gave us a valid id, lets check if it's in the same address space
        KernelLock *kl = locks[id]; //grab the struct

        if (kl != NULL) {
            if (kl->addrSpace != currentThread->space) {
                val = -1; //if not the same address space, return -1
            } else if (kl->isToBeDeleted == true) {
                val = -1;
            } else {
                kl->isToBeDeleted = true; //set it to be deleted
                val = 0; //return 0
            }
        } else {
            val = -1;
        }
    }
    //sysLock.Release();
    */
}

int CreateCondition_Syscall(unsigned int name, int len) {

    //Set up string lock name
    char *buf;		// Kernel buffer for output
    if ( !(buf = new char[len]) ) {
        printf("%s","Error allocating kernel buffer for write!\n");
        return -1;
    } else {
        if ( copyin(name,len,buf) == -1 ) {
            printf("%s","Bad name pointern\n");
            delete[] buf;
            return -1;
        }
    }
    buf[len]='\0';


    //Making syscall message
    stringstream ss;
    ss << SC_CreateCondition << " "<< buf<<"@";
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);

    //Convert the message to int, which will contain either an error code or our new index
    int val = convertMessageToInt(inBuffer);
    return val;

    /*
    KernelCondition *kc = new KernelCondition(); //create new struct
    Condition *c = new Condition(); //create new condition

    kc->condition = c; //assign condition pointer
    kc->addrSpace = currentThread->space; //assign address space

    sysCondition.Acquire();
    conditions.push_back(kc); //add new struct to our conditions vector
    int index = conditions.size() - 1;
    sysCondition.Release();
    return index; //return new index of condition
    */
}

int DestroyCondition_Syscall(int id) {
    int val;
    //sysCondition.Acquire();

    //Making syscall message
    stringstream ss;
    ss << SC_DestroyCondition << " " <<id;
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);
   
    //Convert the message to int
    val = convertMessageToInt(inBuffer);
    return val;

    /*
    if (id > (conditions.size() - 1) || id < 0) {
        val = -1; //if the id they gave us is bad, return -1
    } else { //they gave us a valid id, lets check if it's in the same address space
        KernelCondition *kc = conditions[id]; //grab the struct

        if (kc != NULL) {
            if (kc->addrSpace != currentThread->space) {
                val = -1; //if not the same address space, return -1
            } else if (kc->isToBeDeleted == true) {
                val = -1;
            } else {
                kc->isToBeDeleted = true; //set it to be deleted
                val = 0; //return 0
            }
        } else {
            val = -1;
        }
    }
    //sysCondition.Release();
    */
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
    string = new char[length+1];
    copyin(vaddr,length,string);
    string[length] = '\0';

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

    //currentThread->Finish();

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
    else if(numActiveThreads==1) {
        DEBUG('X', "Process's last thread, deallocating address space\n");
        //Clear all your pages
        for (int i = 0; i < currentThread->space->numPages; i++) {
            freePageBitMap->Clear(currentThread->space->pageTable[i].physicalPage);

        }

        //Clear your locks
        for (int i = 0; i < locks.size(); i++) {
            KernelLock *kl = locks[i]; //grab the struct

            if (kl != NULL) {
                if (kl->addrSpace == currentThread->space && kl->isToBeDeleted == true) {
                    locks[i] = NULL;
                    delete kl;
                }
            }
        }

        //Clear your conditions
        for (int i = 0; i < conditions.size(); i++) {
            KernelCondition *kc = conditions[i]; //grab the struct

            if (kc != NULL) {
                if (kc->addrSpace == currentThread->space && kc->isToBeDeleted == true) {
                    conditions[i] = NULL;
                    delete kc;
                }
            }
        }

        for (int i = 0; i < NumPhysPages; i++) {
            //Clear stuff in the IPT
            if(IPT[i].owner==currentThread->space) {
                IPT[i].valid = FALSE;
            }
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
        //Just clear this threads stack num in the stack bitmap! No need to deal with pages
        //compute stack page numbers
        //int endingPage = divRoundUp(currentThread->baseStackAddr + 16, PageSize);
        int numStackPages = divRoundUp(UserStackSize, PageSize);
        int numCodeDataPages = currentThread->space->numNonStackPages;
        int myStackNum = divRoundUp(divRoundUp(currentThread->baseStackAddr + 16,PageSize) - numCodeDataPages,numStackPages) - 1;
        currentThread->space->stackBitMap.Clear(myStackNum);
        DEBUG('X',"Freeing stack #%d\n",myStackNum);
        /*DEBUG('X',"Freeing stack pages %d through %d\n",endingPage-numStackPages,endingPage);
        for (int i = endingPage - numStackPages + 1; i <= endingPage; i++) {

            //Don't want to actually clear physical memory
            //freePageBitMap->Clear(currentThread->space->pageTable[i].physicalPage);
            currentThread->space->stackBitMap.Clear(i - numCodeDataPages);
        }*/
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

    if (forkArg < 0){
      cout<<"Invalid input to Fork syscall: "<<forkArg<<"\n";
      return;
    }

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

int HandleMemoryFull(){

    DEBUG('M',"Handling memory full\n");

    //Pick a page to evict
    int pageToEvict = rand() % NumPhysPages;


    //Check if that page is in the TLB right now;
    for(int i = 0;i<TLBSize;i++){
        //TODO shallow copy, might get us into trouble
        TranslationEntry tlbEntry = machine->tlb[i];
        if(tlbEntry.valid && tlbEntry.physicalPage==pageToEvict){
            //Now we know we have to copy dirty bits and update pageTable, IPT, invalidate TLB entry
            IPT[pageToEvict].dirty = tlbEntry.dirty;
            IPT[pageToEvict].owner->pageTable[IPT[pageToEvict].virtualPage].dirty = tlbEntry.dirty;
            machine->tlb[i].valid = FALSE;
        }
    }

    DEBUG('M',"Evicting VP %d from PP %d with dirty %d\n",IPT[pageToEvict].virtualPage,pageToEvict,IPT[pageToEvict].dirty);

    //Check dirty bit
    bool dirtyBit = IPT[pageToEvict].dirty;
    if(!dirtyBit){
        //Now we can just overwrite this page, since it's not dirty and it can be read directly from file later
        return pageToEvict;
    }



    //Now we know that we have a dirty page, which is no longer in the TLB
    //We have to copy the page into the swapFIle and note where we put it by modifying the pageTable
    int swapFilePage = swapFileBitMap->Find();
    int swapFileByteOffset = 40 + PageSize*swapFilePage;

    //Write to swap file
    IPT[pageToEvict].owner->pageTable[IPT[pageToEvict].virtualPage].byteOffset = swapFileByteOffset; //store location
    IPT[pageToEvict].owner->pageTable[IPT[pageToEvict].virtualPage].dirty = TRUE; //store info in pageTable
    swapFile->WriteAt(&(machine->mainMemory[pageToEvict * PageSize]), PageSize, swapFileByteOffset); //write file
    DEBUG('M',"Writing to swapfile byte %d\n",swapFileByteOffset);

    return pageToEvict;


    //return 0;

}

void HandlePageFault() {


    IntStatus oldLevel = interrupt->SetLevel(IntOff); //Disable interrupts

    //Get the bad address from register
    int badVAddr = machine->ReadRegister(BadVAddrReg);
    int badPage = badVAddr/PageSize;

    /* This was using the page table of the process, but now we must use
       the IPT to find the page instead since everythign is preloaded

    //Pull out the translation entry from page table
    TranslationEntry old = currentThread->space->pageTable[badPage];
    
    */

    //Will hold the IPT entry that we need
    IPTEntry old;
    bool inIPT = false;

    for(int i = 0; i < NumPhysPages; i++) {
        //get the IPT entry based on virtual page # and address space
        if (IPT[i].valid && IPT[i].virtualPage == badPage && IPT[i].owner == currentThread->space) {
            //TODO This is a shallow copy? Problem?
            old = IPT[i];
            inIPT = true;
            break;
        }
    }

    //Handle IPT miss
    if(!inIPT) {

        //cout<<"Handling IPT miss\n";

        //Get a page of physical memory
        int ppn = freePageBitMap->Find();

        //Handle memory full
        if(ppn==-1){
            ppn = HandleMemoryFull();
        }

        //Check dirty bits and offset to know where to load from
        //Get byte offset, read from executable if it's in there
        int byteOffset = currentThread->space->pageTable[badPage].byteOffset;
        bool loadingDirtyBit = currentThread->space->pageTable[badPage].dirty;
        IPT[ppn].dirty = FALSE;
        //If the page is clean and in the executable, load it
        if(!loadingDirtyBit && badPage<currentThread->space->executableNumPages) {
            //cout<<"Reading from executable\n";
            currentThread->space->processExecutable->ReadAt(&(machine->mainMemory[ppn * PageSize]), PageSize, byteOffset);
            IPT[ppn].dirty = FALSE; //set dirty bit to clean
            DEBUG('M',"Loading from executable byte %d\n",byteOffset);
        }
        else if(loadingDirtyBit){
            //Now the page to be loaded in is dirty, so we must get it from the swap file
            swapFile->ReadAt(&(machine->mainMemory[ppn * PageSize]), PageSize, byteOffset);
            swapFileBitMap->Clear(divRoundUp(byteOffset-40,PageSize));
            IPT[ppn].dirty = TRUE;
            DEBUG('M',"Loading from swapFile byte %d\n",byteOffset);
            //ASSERT(FALSE); //TODO Deal with dirty bit
        }

        // IPT population is here
        IPT[ppn].virtualPage = badPage;
        IPT[ppn].physicalPage = ppn;
        IPT[ppn].owner = currentThread->space;
        IPT[ppn].valid = TRUE;
        IPT[ppn].use = FALSE;
        DEBUG('M',"Loading VP %d to PP %d with dirty %d\n",badPage,ppn,IPT[ppn].dirty);

        IPT[ppn].readOnly = FALSE;

        TranslationEntry* pageTableEntry = &(currentThread->space->pageTable[badPage]);
        pageTableEntry->valid = TRUE;
        pageTableEntry->physicalPage = ppn;
        pageTableEntry->virtualPage = badPage;
        pageTableEntry->dirty = IPT[ppn].dirty;

        //TODO This is a shallow copy? Problem?
        old = IPT[ppn];

    }


    //Before you overwrite, you should copy dirty bits
    if(machine->tlb[currentTLB].valid) {
        bool dirtyBit = machine->tlb[currentTLB].dirty;
        IPT[machine->tlb[currentTLB].physicalPage].dirty = dirtyBit;
        //TODO This was copying the dirty bit to the wrong place
        IPT[machine->tlb[currentTLB].physicalPage].owner->pageTable[machine->tlb[currentTLB].virtualPage].dirty = dirtyBit;
        currentThread->space->pageTable[badPage].dirty = dirtyBit;
    }

    //Add that translation entry to TLB
    machine->tlb[currentTLB].physicalPage = old.physicalPage;
    machine->tlb[currentTLB].virtualPage = old.virtualPage;
    machine->tlb[currentTLB].valid = old.valid;
    machine->tlb[currentTLB].dirty = old.dirty;
    machine->tlb[currentTLB].use = old.use;
    machine->tlb[currentTLB].readOnly = old.readOnly;
    if(!inIPT) {
        DEBUG('M', "Writing to %d TLB a PP %d, VP %d, dirty %d\n", currentTLB, old.physicalPage, old.virtualPage,
              old.dirty);
    }

    //Increment TLB index
    currentTLB = (currentTLB+1)%TLBSize;

    (void) interrupt->SetLevel(oldLevel); //Reenable interrupts

}

int CreateMV_Syscall(unsigned int name, int nameLen, int size){

    //Set up string MV name
    char *buf;		// Kernel buffer for output
    if ( !(buf = new char[nameLen]) ) {
        printf("%s","Error allocating kernel buffer for write!\n");
        return -1;
    } else {
        if ( copyin(name,nameLen,buf) == -1 ) {
            printf("%s","Bad name pointern\n");
            delete[] buf;
            return -1;
        }
    }
    buf[nameLen]='\0';

    //Making syscall message
    stringstream ss;
    ss << SC_CreateMV << " "<< buf<<"@"<<" "<<size;
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);
    
    //Convert the message to int
    int val = convertMessageToInt(inBuffer);
    return val;
}

int DestroyMV_Syscall(int id){

    //Making syscall message
    stringstream ss;
    ss << SC_DestroyMV << " "<< id;
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);

    //Convert the message to int
    int val = convertMessageToInt(inBuffer);
    return val;
}

int SetMV_Syscall(int MVid, int index, int value){

    //Making syscall message
    stringstream ss;
    ss << SC_SetMV << " "<< MVid<<" "<<index<<" "<<value;
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);

    //Convert the message to int
    int val = convertMessageToInt(inBuffer);
    return val;
}

int GetMV_Syscall(int MVid, int index){

    //Making syscall message
    stringstream ss;
    ss << SC_GetMV <<" "<<MVid<<" "<< index;
    char inBuffer[MaxMailSize];
    sendAndRecieveSyscallMessage((char*)ss.str().c_str(),inBuffer);

    //Convert the message to int
    int val = convertMessageToInt(inBuffer);
    return val;
}


void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv = 0;    // the return value from a syscall
    DEBUG('a', "Syscall: %d \n", type);

    if (which == SyscallException) {
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
                Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
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
                rv = CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_DestroyLock:
                DEBUG('a', "DestroyLock syscall.\n");
                rv = DestroyLock_Syscall(machine->ReadRegister(4));
                break;
            case SC_CreateCondition:
                DEBUG('a', "CreateCondition syscall.\n");
                rv = CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
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
                Printf_syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6),
                               machine->ReadRegister(7));
                break;
            case SC_Scanf:
                DEBUG('a', "Scanf syscall.\n");
                rv = Scanf_syscall();
                break;

            case SC_CreateMV:
                DEBUG('a', "CreateMV syscall.\n");
                rv = CreateMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
                break;
            case SC_DestroyMV:
                DEBUG('a', "DestroyMV syscall.\n");
                DestroyMV_Syscall(machine->ReadRegister(4));
                break;
            case SC_SetMV:
                DEBUG('a', "SetMV syscall.\n");
                SetMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
                break;
            case SC_GetMV:
                DEBUG('a', "GetMV syscall.\n");
                rv = GetMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;

        }

        // Put in the return value and increment the PC
        machine->WriteRegister(2, rv);
        machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg) + 4);
        return;
    }
    else if (which == PageFaultException) {
        HandlePageFault();
        return;
    } else {
        cout << "Unexpected user mode exception - which:" << which << "  type:" << type << endl;
        interrupt->Halt();
    }
}
