// synch.cc 
//  Routines for synchronizing threads.  Three kinds of
//  synchronization routines are defined here: semaphores, locks 
//      and condition variables (the implementation of the last two
//  are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  Initialize a semaphore, so that it can be used for synchronization.
//
//  "debugName" is an arbitrary name, useful for debugging.
//  "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  De-allocate semaphore, when no longer needed.  Assume no one
//  is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
//  Wait until semaphore value > 0, then decrement.  Checking the
//  value and decrementing must be done atomically, so we
//  need to disable interrupts before checking the value.
//
//  Note that Thread::Sleep assumes that interrupts are disabled
//  when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    
    while (value == 0) {            // semaphore not available
    queue->Append((void *)currentThread);   // so go to sleep
    currentThread->Sleep();
    } 
    value--;                    // semaphore available, 
                        // consume its value
    
    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//  Increment semaphore value, waking up a waiter if necessary.
//  As with P(), this operation must be atomic, so we need to disable
//  interrupts.  Scheduler::ReadyToRun() assumes that threads
//  are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)    // make thread ready, consuming the V immediately
    scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
//  Lock Functions
//----------------------------------------------------------------------

//  Lock::Lock 
//  Initializes a Lock
//  name is an arbitrary name useful for debugging
//  owner is the Thread that currently owns the Lock
//  lockWaitQueue is the queue of threads waiting to use the lock
//  isFree keeps track of when the lock is available or unavailable
Lock::Lock(char* debugName) {
    name = debugName;
    owner = NULL;
    lockWaitQueue = new List;
    isFree = TRUE;
}

//  Lock::Lock default constructor
Lock::Lock() {
    name = "lock";
    owner = NULL;
    lockWaitQueue = new List;
    isFree = TRUE;
}

//  Lock::~Lock deallocates the Lock when it is no longer needed
Lock::~Lock() {
    delete lockWaitQueue;
}

//  Lock::Acquire
//  Allows a Thread to acquire a lock when the lock is free
void Lock::Acquire() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(isHeldByCurrentThread()){ //current thread already holds this Lock
        (void) interrupt->SetLevel(oldLevel);  // restore interrupts
        return;
    }
    if(isFree){ //lock is available 
        isFree = FALSE; //make state busy
        owner = currentThread;  //make current thread the owner
    }else{ //lock is not available
        lockWaitQueue->Append((void *)currentThread); //put current thread on lock's wait queue
        currentThread->Sleep();
    }
    (void) interrupt->SetLevel(oldLevel);  // restore interrupts
}

//  Lock::Release
//  Makes the Lock available when the Thread is done with it 
void Lock::Release() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(!(isHeldByCurrentThread())){ //current thread does not holds this Lock
        printf("Error: Current thread doesn't have the lock\n");
        (void) interrupt->SetLevel(oldLevel);  // restore interrupts
        return;
    }
    if(!(lockWaitQueue->IsEmpty())){ //lock's wait queue is not empty
        owner = (Thread *)lockWaitQueue->Remove(); //remove one waiting thread and make them lock owner
        scheduler->ReadyToRun(owner); //put thread on back of ready queue
        
    }else{
        isFree = TRUE; //make lock available
        owner = NULL; //unset lock owner
    }
    (void) interrupt->SetLevel(oldLevel);  // restore interrupts
}

//  Lock::isHeldByCurrentThread
//  returns true if the current thread holds this lock
bool Lock::isHeldByCurrentThread(){
    return (owner == currentThread);
}

//----------------------------------------------------------------------
//  Condition Functions
//----------------------------------------------------------------------

//  Condition::Condition
//  Constructor for a condition variable
//  name is an arbitrary name useful for debugging
//  waitingLock is the lock that other waiters gave up
//  waitQueue is a list of threads that are sleeping
Condition::Condition(char* debugName) { 
    name = debugName;
    waitingLock = NULL;
    waitQueue = new List;
}

//Condition::Condition default constructor
Condition::Condition() { 
    name = "CV";
    waitingLock = NULL;
    waitQueue = new List;
}

//  Condition::~Condition
//  Deconstructor for condition variable to clean up after the CV is no longer needed
Condition::~Condition() {
    delete waitQueue;
}

//  Condition::Wait
//  Puts the current thread to sleep
void Condition::Wait(Lock* conditionLock) { 
    //ASSERT(FALSE); 

    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(conditionLock == NULL){  //check for no lock
        printf("Error:No Lock \n");
        (void) interrupt->SetLevel(oldLevel);  // restore interrupts
        return;
    }
    if(waitingLock == NULL){ //no thread waiting
        waitingLock = conditionLock;
    }
    if(waitingLock != conditionLock){ //locks don't match
        printf("Wait Error: Locks don't match \n");
        (void) interrupt->SetLevel(oldLevel);  // restore interrupts
        return; 
    }
    waitQueue->Append((void *)currentThread);// Add current thread to CV wait queue
    conditionLock->Release();   //release the lock before going to sleep
    currentThread->Sleep();     //puts current thread to sleep
    conditionLock->Acquire();   
     (void) interrupt->SetLevel(oldLevel);  // restore interrupts

}

//  Condition::Signal
//  Wakes one sleeping thread if one exists
void Condition::Signal(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(waitingLock != conditionLock && waitingLock!=NULL){ //locks don't match
        printf("Signal Error: Locks don't match \n");
        (void) interrupt->SetLevel(oldLevel);  // restore interrupts
        return;
    }
    if(waitQueue->IsEmpty()){   //no waiting thread exists
        (void) interrupt->SetLevel(oldLevel);  // restore interrupts
        return;
    }
    scheduler->ReadyToRun((Thread*)waitQueue->Remove()); //wake waiting thread from queue and put on ready queue
    if(waitQueue->IsEmpty()){ //make lock null when no other waiting thread exists
        waitingLock = NULL;
    }
    (void) interrupt->SetLevel(oldLevel);  // restore interrupts
}

//  Condition::Broadcast
//  Wakes all sleeping threads
void Condition::Broadcast(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(conditionLock == NULL){  //Checks if there's no lock
        printf("Error: No Lock\n");
        (void) interrupt->SetLevel(oldLevel);  // restore interrupts
        return;
    }
    if(waitingLock != conditionLock){ //Checks if locks don't match
        if(waitingLock!=NULL) {
            printf("Error: Locks don't match \n");
        }
        (void) interrupt->SetLevel(oldLevel);  // restore interrupts
        return;
    }
    (void) interrupt->SetLevel(oldLevel);  // restore interrupts
    while(!(waitQueue->IsEmpty())){ // signals all the sleeping threads in the wait queue until there are no more threads waiting
        Signal(conditionLock);
    }
}
