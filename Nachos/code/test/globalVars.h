#include "monitor.h"

#ifndef GLOBALVARS_H
#define GLOBALVARS_H

extern struct Monitor appClerk, picClerk, passPClerk, cashier;
extern int numCustomersLeft;
extern int bribesEnabled;

/*global shared data between the clerks that are used for filing purposes */
extern int customersWithCompletedApps;
extern int customersWithCompletedPics;
extern int passportClerkChecked;
extern int cashierChecked;
extern int gottenPassport;
extern int cashReceived;

extern int appClerkCurrentCustomer;
extern int pictureClerkCurrentCustomer;
extern int passportClerkCurrentCustomer;
extern int cashierCurrentCustomer;

extern int senatorLock;
extern int senatorCV;
extern int isSenator;
extern int senatorWorking;
extern int clerksCanWork;

extern int newCustomerId;
extern int newCustomerIdLock;

#endif
