#ifndef MONITOR_H
#define MONITOR_H

/*global struct that contains all locks, monitor variables, and condition
variables needed to properly implement synchronization*/
struct Monitor {
    
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




