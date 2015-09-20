//
// Created by meldefon on 9/13/15.
//

#ifndef MONITOR_H
#define MONITOR_H


//global struct that contains all locks, monitor variables, and condition
//variables needed to properly implement synchronization
struct Monitor {

    Monitor(){
        numOfClerks=0;
    }
    void initialize(char* lockName,char* clerkType_, int size) {
        lineLock = new Lock(lockName);
        lineCV = new Condition[size]();
        clerkLock = new Lock[size]();
        clerkCV = new Condition[size]();
        lineCount = new int[size];
        limboLineCV = new Condition();
        bribeLineCount = new int[size];
        bribeLineCV = new Condition[size]();
        clerkState = new int[size];
        numOfClerks = size;
        clerkType = clerkType_;
        breakCV = new Condition();
        currentCustomer = new int[size];
        numCustomersInLimbo = 0;

        for(int i = 0; i < size; i++) {
            lineCount[i] = 0;
            bribeLineCount[i] = 0;
            clerkState[i] = 0;
            currentCustomer[i] = -1;
        }
    }

    Lock *lineLock;
    Condition *lineCV;
    Condition *bribeLineCV;
    Condition *limboLineCV;
    Lock *clerkLock;
    Condition *clerkCV;
    Condition *breakCV;
    int *lineCount;
    int *bribeLineCount;
    int *clerkState;
    int numOfClerks;
    char* clerkType;
    int cashReceived;
    int *currentCustomer;
    int numCustomersInLimbo;
};


#endif //MONITOR_H