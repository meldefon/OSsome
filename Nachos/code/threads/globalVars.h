//
// Created by meldefon on 9/13/15.
//

#include "monitor.h"

#ifndef GLOBALVARS_H
#define GLOBALVARS_H



extern Monitor appClerk, picClerk, passPClerk, cashier;

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


#endif //GLOBALVARS_H
