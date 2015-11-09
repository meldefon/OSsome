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
    int lockID, mvID, val;

    /*Create the lock*/
    lockID = CreateLock("testLock",8);
    mvID = CreateMV("testMV",6,1);

    /*Accuire*/
    Acquire(lockID);

    Write("Getting and incrementing MV...\n",31,ConsoleOutput);

    val = GetMV(mvID,0);
    Printf("Val before: %d\n",15,val*100000,0);
    SetMV(mvID,0,val+1);

    Printf("Val after: %d\n",14,(val+1)*100000,0);

    /*Release*/
    Release(lockID);

    Write("Out of critical section\n",24,ConsoleOutput);

    /*Destroy the lock (really, set its toBeDestroyed bit to true*/
    DestroyLock(lockID);
    DestroyMV(mvID);


    /*Send a result to print*/
    Exit(0);

}