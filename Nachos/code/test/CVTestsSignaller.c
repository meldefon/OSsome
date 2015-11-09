/* CVTests.c
 * Will test the server implementation of the CV syscalls
 * Create: makes a new CV, or gives the id of the existing CV with that name
 * Destroy: sets CV destroy bit to true, which is cleaned up at a later safe time
 * Wait: waits on CV until a signal or broadcast
 * Signal: signals a waiting thread
 * Broadcast: signals all waiting threads
 * */


#include "syscall.h"

#define LEN 100

int
main() {

    /*Create lock, CV*/
    int lockID, cvID;
    lockID = CreateLock("testLock",8);
    cvID = CreateCondition("testCV",6);


    /*Get into critical section*/
    Acquire(lockID);

    Write("About to signal\n",16,ConsoleOutput);

    /*Signal*/
    Signal(cvID,lockID);

    /*Release*/
    Release(lockID);

    /*Destroy the lock, cv (really, set its toBeDestroyed bit to true)*/
    DestroyLock(lockID);
    DestroyCondition(cvID);

    /*Send a result to print*/
    Exit(0);



}
