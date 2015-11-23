#include "monitor.h"
#include "globalVars.h"
#include "customer.c"

#define NULL 0

void createServerMVs(int numCustomers, int numberOfSenators) {
	int i;
	/*global shared data between the clerks that are used for filing purposes */
	numCustomersLeft = CreateMV("numCustomersLeft", 17, 1);
	bribesEnabled = CreateMV("bribesEnabled", 14, 1);

	customersWithCompletedApps = CreateMV("customersWithCompletedApps", 27, numCustomers);
	customersWithCompletedPics = CreateMV("customersWithCompletedPics", 27, numCustomers);
	passportClerkChecked = CreateMV("passportClerkChecked", 21, numCustomers);
	cashierChecked = CreateMV("cashierChecked", 15, numCustomers);
	gottenPassport = CreateMV("gottenPassport", 15, numCustomers);
	cashReceived = CreateMV("cashReceived", 13, numCustomers);

	appClerkCurrentCustomer = CreateMV("appClerkCurrentCustomer", 24, numCustomers);
	pictureClerkCurrentCustomer = CreateMV("pictureClerkCurrentCustomer", 28, numCustomers);
	passportClerkCurrentCustomer = CreateMV("passportClerkCurrentCustomer", 29, numCustomers);
	cashierCurrentCustomer = CreateMV("cashierCurrentCustomer", 23, numCustomers);

	senatorLock = CreateLock("senatorLock", 12);
	senatorCV = CreateCondition("senatorCV", 10);
	isSenator = CreateMV("isSenator", 10, numCustomers);
	senatorWorking = CreateMV("senatorWorking", 15, 1);
	clerksCanWork = CreateMV("clerksCanWork", 14, 1);

	newCustomerId = CreateMV("newCustomerId", 14, 1);
	newCustomerIdLock = CreateLock("newCustomerIdLock", 18);

	/*Initialize everything*/
		
	/*
	numCustomersLeft = size;
	bribesEnabled = 1;
	newCustomerId = 0;
	*/

	SetMV(numCustomersLeft, 0, numCustomers);
	SetMV(bribesEnabled, 0, 1);
	SetMV(newCustomerId, 0, 0);

	for(i = 0; i < numCustomers; i++) {

		/*
		customersWithCompletedApps[i] = 0;
		customersWithCompletedPics[i] = 0;
		passportClerkChecked[i] = 0;
		cashierChecked[i] = 0;
		gottenPassport[i] = 0;
		cashReceived[i] = 0;
		*/

		SetMV(customersWithCompletedApps, i, 0);
		SetMV(customersWithCompletedPics, i, 0);
		SetMV(passportClerkChecked, i, 0);
		SetMV(cashierChecked, i, 0);
		SetMV(gottenPassport, i, 0);
		SetMV(cashReceived, i, 0);

		if(i < numCustomers-numberOfSenators) {
			SetMV(isSenator, i, 0); /*isSenator[i]= 0;*/
		} else {
			SetMV(isSenator, i, 1);/*isSenator[i]= 1;*/
		}
	}

	/* 
	senatorWorking = NULL;
	clerksCanWork = 1;
	*/

	SetMV(senatorWorking, 0, NULL);
	SetMV(clerksCanWork, 0, 1);
}

void initialize(struct Monitor *m, int clerkType_, int size) {
	int i;
	char* lockName;
	int nameLength;

	nameLength = 50;

	if(clerkType_ == 0) {
		m->lineLock = CreateLock("lineLock0", 10);
		m->newClerkIdLock = CreateLock("newClerkIdLock0", 16);					
		m->newClerkId = CreateMV("newClerkId0", 12, 1);
		m->clerkType = CreateMV("clerkType0", 11, 1);
		m->lineCV = CreateMV("lineCV0", 8, size);
		m->bribeLineCV = CreateMV("bribeLineCV0", 13, size);
		m->senLineCV = CreateMV("senLineCV0", 11, size);
		m->limboLineCV = CreateMV("limboLineCV0", 13, size);
		m->clerkLock = CreateMV("clerkLock0", 11, size);
		m->clerkCV = CreateMV("clerkCV0", 9, size);
		m->breakCV = CreateCondition("breakCV0", 9);
		m->lineCount = CreateMV("lineCount0", 11, size);
		m->bribeLineCount = CreateMV("bribeLineCount0", 16, size);
		m->senLineCount = CreateMV("senLineCount0", 14, size);
		m->clerkState = CreateMV("clerkState0", 12, size);
		m->numOfClerks = CreateMV("numOfClerks0", 13, 1);
		m->cashReceived = CreateMV("cashReceived0", 14, 1);
		m->currentCustomer = CreateMV("currentCustomer0", 17, size);
		m->numCustomersInLimbo = CreateMV("numCustomersInLimbo0", 21, 1);
		lockName = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\0";
	} else if(clerkType_ == 1) {
		m->lineLock = CreateLock("lineLock1", 10);
		m->newClerkIdLock = CreateLock("newClerkIdLock1", 16);											
		m->newClerkId = CreateMV("newClerkId1", 12, 1);
		m->clerkType = CreateMV("clerkType1", 11, 1);
		m->lineCV = CreateMV("lineCV1", 8, size);
		m->bribeLineCV = CreateMV("bribeLineCV1", 13, size);
		m->senLineCV = CreateMV("senLineCV1", 11, size);
		m->limboLineCV = CreateMV("limboLineCV0", 13, size);
		m->clerkLock = CreateMV("clerkLock1", 11, size);
		m->clerkCV = CreateMV("clerkCV1", 9, size);
		m->breakCV = CreateCondition("breakCV1", 9);
		m->lineCount = CreateMV("lineCount1", 11, size);
		m->bribeLineCount = CreateMV("bribeLineCount1", 16, size);
		m->senLineCount = CreateMV("senLineCount1", 14, size);
		m->clerkState = CreateMV("clerkState1", 12, size);
		m->numOfClerks = CreateMV("numOfClerks1", 13, 1);
		m->cashReceived = CreateMV("cashReceived1", 14, 1);
		m->currentCustomer = CreateMV("currentCustomer1", 17, size);
		m->numCustomersInLimbo = CreateMV("numCustomersInLimbo1", 21, 1);
		lockName = "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\0";
	} else if(clerkType_ == 2) {
		m->lineLock = CreateLock("lineLock2", 10);
		m->newClerkIdLock = CreateLock("newClerkIdLock2", 16);											
		m->newClerkId = CreateMV("newClerkId2", 12, 1);
		m->clerkType = CreateMV("clerkType2", 11, 1);
		m->lineCV = CreateMV("lineCV2", 8, size);
		m->bribeLineCV = CreateMV("bribeLineCV2", 13, size);
		m->senLineCV = CreateMV("senLineCV2", 11, size);
		m->limboLineCV = CreateMV("limboLineCV2", 13, size);
		m->clerkLock = CreateMV("clerkLock2", 11, size);
		m->clerkCV = CreateMV("clerkCV2", 9, size);
		m->breakCV = CreateCondition("breakCV2", 9);
		m->lineCount = CreateMV("lineCount2", 11, size);
		m->bribeLineCount = CreateMV("bribeLineCount2", 16, size);
		m->senLineCount = CreateMV("senLineCount2", 14, size);
		m->clerkState = CreateMV("clerkState2", 12, size);
		m->numOfClerks = CreateMV("numOfClerks2", 13, 1);
		m->cashReceived = CreateMV("cashReceived2", 14, 1);
		m->currentCustomer = CreateMV("currentCustomer2", 17, size);
		m->numCustomersInLimbo = CreateMV("numCustomersInLimbo2", 21, 1);
		lockName = "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\0";		
	} else if(clerkType_ == 3) {
		m->lineLock = CreateLock("lineLock3", 10);
		m->newClerkIdLock = CreateLock("newClerkIdLock3", 16);											
		m->newClerkId = CreateMV("newClerkId3", 12, 1);
		m->clerkType = CreateMV("clerkType3", 11, 1);
		m->lineCV = CreateMV("lineCV3", 8, size);
		m->bribeLineCV = CreateMV("bribeLineCV3", 13, size);
		m->senLineCV = CreateMV("senLineCV3", 11, size);
		m->limboLineCV = CreateMV("limboLineCV3", 13, size);
		m->clerkLock = CreateMV("clerkLock3", 11, size);
		m->clerkCV = CreateMV("clerkCV3", 9, size);
		m->breakCV = CreateCondition("breakCV3", 9);
		m->lineCount = CreateMV("lineCount3", 11, size);
		m->bribeLineCount = CreateMV("bribeLineCount3", 16, size);
		m->senLineCount = CreateMV("senLineCount3", 14, size);
		m->clerkState = CreateMV("clerkState3", 12, size);
		m->numOfClerks = CreateMV("numOfClerks3", 13, 1);
		m->cashReceived = CreateMV("cashReceived3", 14, 1);
		m->currentCustomer = CreateMV("currentCustomer3", 17, size);
		m->numCustomersInLimbo = CreateMV("numCustomersInLimbo3", 21, 1);
		lockName = "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD\0";
	}
	
	SetMV(m->newClerkId, 0, 0);
	SetMV(m->numOfClerks, 0, size);
	SetMV(m->clerkType, 0, clerkType_);
	SetMV(m->numCustomersInLimbo, 0, 0);
	SetMV(m->cashReceived, 0, 0);

	for (i = 0; i < size; i++) {
		/*
		m->lineCV[i] = CreateCondition(lockName, nameLength);
		m->clerkLock[i] = CreateLock(lockName, nameLength);
		m->clerkCV[i] = CreateCondition(lockName, nameLength);
		m->bribeLineCV[i] = CreateCondition(lockName, nameLength);
		m->senLineCV[i] = CreateCondition(lockName, nameLength);
		*/

		SetMV(m->lineCV, i, CreateCondition(lockName, nameLength));
		SetMV(m->clerkLock, i, CreateCondition(lockName, nameLength));
		nameLength--;
		SetMV(m->clerkCV, i, CreateCondition(lockName, nameLength));
		nameLength--;
		SetMV(m->bribeLineCount, i, CreateCondition(lockName, nameLength));
		nameLength--;
		SetMV(m->senLineCV, i, CreateCondition(lockName, nameLength));
		nameLength--;

		/*
		m->lineCount[i] = 0;
		m->bribeLineCount[i] = 0;
		m->senLineCount[i] = 0;
		m->clerkState[i] = 0;
		m->currentCustomer[i] = -1;
		*/

		SetMV(m->lineCount, i, 0);
		SetMV(m->bribeLineCount, i, 0);
		SetMV(m->senLineCount, i, 0);
		SetMV(m->clerkState, i, 0);
		SetMV(m->currentCustomer, i, -1);
	}
}

int main() {

  	int size; /*will be used to take in user input for the sizes of specific variables*/
  	int senatorSize;
	int i;
    int j;
    int k;
    int l;
    int m;
    int n;
	int numberOfSenators;
	int testSuite;
	int numAppClerks;
	int numPicClerks;
	int numCashiers;
	int numPassportClerks;

	Uprintf("For TestSuite, enter 1\nFor Simulation, enter 2", 46, 0, 0, 0, 0);
	testSuite = Scanf();

	if(testSuite == 1)
		testSuite = 0; /*TestSuite();*/
	else {

		Uprintf("Number of ApplicationClerks = ", 30, 0, 0, 0, 0);
		size = Scanf();
		numAppClerks = size;

		initialize(&appClerk, 0, size);

		Uprintf("Number of PictureClerks = ", 26, 0, 0, 0, 0);
		size = Scanf();
		numPicClerks = size;

		initialize(&picClerk, 1, size);

		Uprintf("Number of PassportClerks = ", 27, 0, 0, 0, 0);
		size = Scanf();
		numPassportClerks = size;

		initialize(&passPClerk, 2, size);
		
		Uprintf("Number of Cashiers = ", 21, 0, 0, 0, 0);
		size = Scanf();
		numCashiers = size;

		initialize(&cashier, 3, size);
		
		Uprintf("Number of Customers = ", 22, 0, 0, 0, 0);
		size = Scanf();

		Uprintf("Number of Senators = ", 21, 0, 0, 0, 0);
		numberOfSenators = Scanf();
		
		size += numberOfSenators;

		/* Create global server MVs and initialize everything */
		createServerMVs(size, numberOfSenators);

		/*initialize all the programs here*/ 
		for(j = 0; j < size; j++) {
    		Exec("../test/customer",16);
		}

		for(k = 0; k < numAppClerks; k++) {
    		Exec("../test/appClerk",16);
		}

		for(l = 0; l < numPicClerks; l++) {
    		Exec("../test/picClerk",16);
		}

		for(m = 0; m < numPassportClerks; m++) {
    		Exec("../test/passPClerk",18);
		}

		for(n = 0; n < numCashiers; n++) {
    		Exec("../test/cashier",15);
		}

    	Exec("../test/manager",15);
		
		return;
	}
}
