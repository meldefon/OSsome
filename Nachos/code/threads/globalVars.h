//
// Created by meldefon on 9/13/15.
//

#include "monitor.h"

#ifndef GLOBALVARS_H
#define GLOBALVARS_H



extern Monitor appClerk, picClerk, passPClerk, cashier;
extern int numCustomersLeft;
extern bool bribesEnabled;

//global shared data between the clerks that are used for filing purposes
extern bool *customersWithCompletedApps;
extern bool *customersWithCompletedPics;
extern bool *passportClerkChecked;
extern bool *cashierChecked;
extern bool *gottenPassport;
extern int *cashReceived;

extern int* appClerkCurrentCustomer;
extern int* pictureClerkCurrentCustomer;
extern int* passportClerkCurrentCustomer;
extern int* cashierCurrentCustomer;

extern Lock* senatorLock;
extern Condition* senatorCV;
extern bool* isSenator;
extern int senatorWorking;

extern bool clerksCanWork;


#endif //GLOBALVARS_H
