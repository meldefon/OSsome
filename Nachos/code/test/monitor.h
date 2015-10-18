#ifndef MONITOR_H
#define MONITOR_H

/*global struct that contains all locks, monitor variables, and condition
variables needed to properly implement synchronization*/
struct Monitor {

    Monitor(){
        numOfClerks=0;
    }
    void initialize(char* lockName,char* clerkType_, int size) {
        lineLock = CreateLock();
        /*lineCV = (int*) malloc(size * sizeof(int));
        clerkLock = (int*) malloc(size * sizeof(int));
        clerkCV = (int*) malloc(size * sizeof(int));
        lineCount = (int*) malloc(size * sizeof(int));
        limboLineCV = CreateCondition();
        bribeLineCount = (int*) malloc(size * sizeof(int));
        bribeLineCV = (int*) malloc(size * sizeof(int));
        senLineCV = (int*) malloc(size * sizeof(int));
        senLineCount = (int*) malloc(size * sizeof(int));
        clerkState = (int*) malloc(size * sizeof(int)); 
        currentCustomer = (int*) malloc(size * sizeof(int));*/
        numOfClerks = size;
        clerkType = clerkType_;
        breakCV = CreateCondition();
        numCustomersInLimbo = 0;
        cashReceived = 0;

        for(int i = 0; i < size; i++) {
            lineCV[i] = CreateCondition();
            clerkLock[i] = CreateLock(); 
            clerkCV[i] = CreateCondition();    
            bribeLineCV[i] = CreateCondition();    
            senLineCV[i] = CreateCondition();    

            lineCount[i] = 0;
            bribeLineCount[i] = 0;
            senLineCount[i] = 0;
            clerkState[i] = 0;
            currentCustomer[i] = -1;
        }
    }

    int lineLock;
    int lineCV[50];
    int bribeLineCV[50];
    int senLineCV[50];
    int limboLineCV;
    int clerkLock[50];
    int clerkCV[50];
    int breakCV;
    int lineCount[50];
    int bribeLineCount[50];
    int senLineCount[50];
    int clerkState[50];
    int numOfClerks;
    char* clerkType;
    int cashReceived;
    int currentCustomer[50];
    int numCustomersInLimbo;
};

#endif 




