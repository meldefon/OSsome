//
// Created by meldefon on 9/13/15.
//

#ifndef OSSOME_GLOBALVARS_H
#define OSSOME_GLOBALVARS_H



//global struct that contains all locks, monitor variables, and condition
//variables needed to properly implement synchronization
struct Monitor {
    Lock *lineLock;
    Condition *lineCV;
    Lock *clerkLock;
    Condition *clerkCV;
    int *lineCount;
    int *bribeLineCount;
    int *clerkState;
} appClerk, picClerk, passPClerk, cashier;

//global shared data between the clerks that are used for filing purposes
bool *customersWithCompletedApps;
bool *customersWithCompletedPics;
bool *passportClerkChecked;
bool *gottenPassport;
int *cashReceived;



#endif //OSSOME_GLOBALVARS_H
