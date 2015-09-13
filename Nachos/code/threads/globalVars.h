//
// Created by meldefon on 9/13/15.
//

#include "monitor.h"

#ifndef OSSOME_GLOBALVARS_H
#define OSSOME_GLOBALVARS_H



extern Monitor appClerk, picClerk, passPClerk, cashier;

//global shared data between the clerks that are used for filing purposes
extern bool *customersWithCompletedApps;
extern bool *customersWithCompletedPics;
extern bool *passportClerkChecked;
extern bool *gottenPassport;
extern int *cashReceived;



#endif //OSSOME_GLOBALVARS_H
