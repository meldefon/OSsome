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
    int i,j,total;
    int lockID, cvID;
    lockID = CreateLock("testLock",8);
    cvID = CreateCondition("testCV",6);


    /*Get into critical section*/
    Acquire(lockID);

    Write("Sector 1...\n",12,ConsoleOutput);

    /*Do some work*/
    for(i = 0;i<LEN;i++){
        for(j = 0;j<LEN;j++){
            total += i*j; /*Just to keep the thread occupied*/
        }
    }

    /*Signal and then wait for another signal*/
    Signal(cvID,lockID);
    Wait(cvID,lockID);

    Write("Sector 2...\n",12,ConsoleOutput);

    /*Do some work*/
    for(i = 0;i<LEN;i++){
        for(j = 0;j<LEN;j++){
            total += i*j; /*Just to keep the thread occupied*/
        }
    }

    /*Signal. Even though the last thread will be signalling no one, this is part of the test
     * to allow all threads to finish*/
    Signal(cvID,lockID);

    /*Release*/
    Release(lockID);

    Write("Out of critical section\n",24,ConsoleOutput);

    /*Destroy the lock, cv (really, set its toBeDestroyed bit to true)*/
    DestroyLock(lockID);
    DestroyCondition(cvID);

    /*Send a result to print*/
    Exit(total);



}
