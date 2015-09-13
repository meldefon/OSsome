//
// Created by meldefon on 9/13/15.
//

#ifndef OSSOME_MONITOR_H
#define OSSOME_MONITOR_H


//global struct that contains all locks, monitor variables, and condition
//variables needed to properly implement synchronization
struct Monitor {

    Monitor(){
        numOfClerks=0;
    }
    Monitor(char* lockName, int size) {
        lineLock = new Lock(lockName);
        lineCV = new Condition[size]();
        clerkLock = new Lock[size]();
        clerkCV = new Condition[size]();
        lineCount = new int[size]();
        bribeLineCount = new int[size]();
        clerkState = new int[size]();
        numOfClerks = size;
    }
    Lock *lineLock;
    Condition *lineCV;
    Lock *clerkLock;
    Condition *clerkCV;
    int *lineCount;
    int *bribeLineCount;
    int *clerkState;
    int numOfClerks;
};


#endif //OSSOME_MONITOR_H
