//
// Created by meldefon on 9/13/15.
//

#include "monitor.h"

#ifndef GLOBALVARS_H
#define GLOBALVARS_H

extern Monitor appClerk, picClerk, passPClerk, cashier;
extern int numCustomersLeft;
extern bool bribesEnabled;
//extern int bribesEnabled;

//global shared data between the clerks that are used for filing purposes
extern bool *customersWithCompletedApps;
//extern int *customersWithCompletedApps;
extern bool *customersWithCompletedPics;
//extern int *customersWithCompletedPics;
extern bool *passportClerkChecked;
//extern int *passportClerkChecked;
extern bool *cashierChecked;
//extern int *cashierChecked;
extern bool *gottenPassport;
//extern int *gottenPassport;
extern int *cashReceived;

extern int* appClerkCurrentCustomer;
extern int* pictureClerkCurrentCustomer;
extern int* passportClerkCurrentCustomer;
extern int* cashierCurrentCustomer;

extern Lock* senatorLock;
//extern int senatorLock;
extern Condition* senatorCV;
//extern int senatorCV;
extern bool* isSenator;
//extern *isSenator;
extern int senatorWorking;

extern bool clerksCanWork;
//extern int clerksCanWork;

#endif //GLOBALVARS_H
