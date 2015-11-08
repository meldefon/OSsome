/* lockTests.c
 * Will test the server implementation of the lock syscalls
 * Create: makes a new lock, or gives the id of the existing lock with that name
 * Destroy: sets lock destroy bit to true, which is cleaned up at a later safe time
 * Acquire: acquires a lock for mutual exclusion
 * Release: releases a lock after critical section code
 * */

#include "syscall.h"

#define LEN 100

int
main() {

    int i, j;
    int lockID, total;

    /*Create the lock*/
    lockID = CreateLock("testLock",8);

    /*Accuire*/
    Acquire(lockID);

    Write("Working...\n",11,ConsoleOutput);

    /*Do some work*/
    for(i = 0;i<LEN;i++){
        for(j = 0;j<LEN;j++){
            total += i*j; /*Just to keep the thread occupied*/
        }
    }

    /*Release*/
    Release(lockID);

    Write("Out of critical section\n",24,ConsoleOutput);

    /*Destroy the lock (really, set its toBeDestroyed bit to true*/
    DestroyLock(lockID);

    /*Send a result to print*/
    Exit(total);

}