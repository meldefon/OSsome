#include "monitor.h"

#ifndef GLOBALVARS_H
#define GLOBALVARS_H

extern struct Monitor appClerk, picClerk, passPClerk, cashier;
extern int numCustomersLeft;
extern int bribesEnabled;

/*global shared data between the clerks that are used for filing purposes */
extern int customersWithCompletedApps[5];
extern int customersWithCompletedPics[5];
extern int passportClerkChecked[5];
extern int cashierChecked[5];
extern int gottenPassport[5];
extern int cashReceived[5];

extern int appClerkCurrentCustomer[5];
extern int pictureClerkCurrentCustomer[5];
extern int passportClerkCurrentCustomer[5];
extern int cashierCurrentCustomer[5];

extern int senatorLock;
extern int senatorCV;
extern int isSenator[5];
extern int senatorWorking;
extern int clerksCanWork;

extern int newCustomerId;
extern int newCustomerIdLock;

#endif

