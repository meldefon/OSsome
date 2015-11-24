#include "monitor.h"

#ifndef SETUP_H
#define SETUP_H

/* Function prototypes for initializing and creating MVs */
void createServerMVs(int numCustomers, int numberOfSenators);

void initialize(struct Monitor *m, int clerkType_, int size);

#endif


