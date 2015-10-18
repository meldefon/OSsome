#include "monitor.h"

#ifndef GLOBALVARS_H
#define GLOBALVARS_H

extern typedef struct Monitor appClerk, picClerk, passPClerk, cashier;
extern int numCustomersLeft;
extern int bribesEnabled;

/*global shared data between the clerks that are used for filing purposes */
extern int customersWithCompletedApps[50];
extern int customersWithCompletedPics[50];
extern int passportClerkChecked[50];
extern int cashierChecked[50];
extern int gottenPassport[50];
extern int cashReceived[50];

extern int appClerkCurrentCustomer[50];
extern int pictureClerkCurrentCustomer[50];
extern int passportClerkCurrentCustomer[50];
extern int cashierCurrentCustomer[50];

extern int senatorLock;
extern int senatorCV;
extern int isSenator[50];
extern int senatorWorking;
extern int clerksCanWork;

#endif

