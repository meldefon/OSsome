#ifndef MONITOR_H
#define MONITOR_H

/*global struct that contains all locks, monitor variables, and condition
variables needed to properly implement synchronization*/
struct Monitor {
    
    int lineLock;
    int lineCV; /* int lineCV[50]; */
    int bribeLineCV; /* int bribeLineCV[50]; */
    int senLineCV; /*int senLineCV[50];*/
    int limboLineCV;
    int clerkLock; /*int clerkLock[50];*/
    int clerkCV; /*int clerkCV[50];*/
    int breakCV;
    int lineCount; /*int lineCount[50];*/
    int bribeLineCount; /*int bribeLineCount[50];*/
    int senLineCount; /*int senLineCount[50];*/
    int clerkState; /*int clerkState[50];*/
    int numOfClerks;
    int clerkType;
    int cashReceived;
    int currentCustomer; /*int currentCustomer[50];*/
    int numCustomersInLimbo;
    int newClerkId;
    int newClerkIdLock;
};

#endif 




