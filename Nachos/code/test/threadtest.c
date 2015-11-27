#include "monitor.h"
#define NULL 0

struct Monitor appClerk, picClerk, passPClerk, cashier;

/*global shared data between the clerks that are used for filing purposes */
int customersWithCompletedApps;
int customersWithCompletedPics;
int passportClerkChecked;
int cashierChecked;
int gottenPassport;
int cashReceived;
int bribesEnabled;

int appClerkCurrentCustomer;
int pictureClerkCurrentCustomer;
int passportClerkCurrentCustomer;
int cashierCurrentCustomer;

int senatorLock;
int senatorCV;
int isSenator;
int senatorWorking;
int clerksCanWork;

int numCustomersLeft;
int newCustomerId;
int newCustomerIdLock;

void Uprintf(char *string, int length, int num_1, int num_2, int num_3, int num_4) {
    Printf(string, length, (num_1 * 100000) + num_2, (num_3 * 100000) + num_4);
}

void createServerMVs(int numCustomers, int numberOfSenators) {
	int i;
	/*global shared data between the clerks that are used for filing purposes */
	numCustomersLeft = CreateMV("numCustomersLeft", 16, 1);
	bribesEnabled = CreateMV("bribesEnabled", 13, 1);

	customersWithCompletedApps = CreateMV("customersWithCompletedApps", 26, numCustomers);
	customersWithCompletedPics = CreateMV("customersWithCompletedPics", 26, numCustomers);
	passportClerkChecked = CreateMV("passportClerkChecked", 20, numCustomers);
	cashierChecked = CreateMV("cashierChecked", 14, numCustomers);
	gottenPassport = CreateMV("gottenPassport", 14, numCustomers);
	cashReceived = CreateMV("cashReceived", 12, numCustomers);

	appClerkCurrentCustomer = CreateMV("appClerkCurrentCustomer", 23, numCustomers);
	pictureClerkCurrentCustomer = CreateMV("pictureClerkCurrentCustomer", 27, numCustomers);
	passportClerkCurrentCustomer = CreateMV("passportClerkCurrentCustomer", 28, numCustomers);
	cashierCurrentCustomer = CreateMV("cashierCurrentCustomer", 22, numCustomers);

	senatorLock = CreateLock("senatorLock", 11);
	senatorCV = CreateCondition("senatorCV", 9);
	isSenator = CreateMV("isSenator", 9, numCustomers);
	senatorWorking = CreateMV("senatorWorking", 14, 1);
	clerksCanWork = CreateMV("clerksCanWork", 13, 1);

	newCustomerId = CreateMV("newCustomerId", 13, 1);
	newCustomerIdLock = CreateLock("newCustomerIdLock", 17);

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
		m->lineLock = CreateLock("lineLock0", 9);
		m->newClerkIdLock = CreateLock("newClerkIdLock0", 15);
		m->newClerkId = CreateMV("newClerkId0", 11, 1);
		m->clerkType = CreateMV("clerkType0", 10, 1);
		m->lineCV = CreateMV("lineCV0", 7, size);
		m->bribeLineCV = CreateMV("bribeLineCV0", 12, size);
		m->senLineCV = CreateMV("senLineCV0", 10, size);
		m->limboLineCV = CreateMV("limboLineCV0", 12, size);
		m->clerkLock = CreateMV("clerkLock0", 10, size);
		m->clerkCV = CreateMV("clerkCV0", 8, size);
		m->breakCV = CreateCondition("breakCV0", 8);
		m->lineCount = CreateMV("lineCount0", 10, size);
		m->bribeLineCount = CreateMV("bribeLineCount0", 15, size);
		m->senLineCount = CreateMV("senLineCount0", 13, size);
		m->clerkState = CreateMV("clerkState0", 11, size);
		m->numOfClerks = CreateMV("numOfClerks0", 12, 1);
		m->cashReceived = CreateMV("cashReceived0", 13, 1);
		m->currentCustomer = CreateMV("currentCustomer0", 17, size);
		m->numCustomersInLimbo = CreateMV("numCustomersInLimbo0", 20, 1);
		lockName = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
	} else if(clerkType_ == 1) {
		m->lineLock = CreateLock("lineLock1", 9);
		m->newClerkIdLock = CreateLock("newClerkIdLock1", 15);
		m->newClerkId = CreateMV("newClerkId1", 11, 1);
		m->clerkType = CreateMV("clerkType1", 10, 1);
		m->lineCV = CreateMV("lineCV1", 7, size);
		m->bribeLineCV = CreateMV("bribeLineCV1", 12, size);
		m->senLineCV = CreateMV("senLineCV1", 10, size);
		m->limboLineCV = CreateMV("limboLineCV0", 12, size);
		m->clerkLock = CreateMV("clerkLock1", 10, size);
		m->clerkCV = CreateMV("clerkCV1", 8, size);
		m->breakCV = CreateCondition("breakCV1", 8);
		m->lineCount = CreateMV("lineCount1", 10, size);
		m->bribeLineCount = CreateMV("bribeLineCount1", 15, size);
		m->senLineCount = CreateMV("senLineCount1", 13, size);
		m->clerkState = CreateMV("clerkState1", 11, size);
		m->numOfClerks = CreateMV("numOfClerks1", 12, 1);
		m->cashReceived = CreateMV("cashReceived1", 13, 1);
		m->currentCustomer = CreateMV("currentCustomer1", 16, size);
		m->numCustomersInLimbo = CreateMV("numCustomersInLimbo1", 20, 1);
		lockName = "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB";
	} else if(clerkType_ == 2) {
		m->lineLock = CreateLock("lineLock2", 9);
		m->newClerkIdLock = CreateLock("newClerkIdLock2", 15);
		m->newClerkId = CreateMV("newClerkId2", 11, 1);
		m->clerkType = CreateMV("clerkType2", 10, 1);
		m->lineCV = CreateMV("lineCV2", 7, size);
		m->bribeLineCV = CreateMV("bribeLineCV2", 12, size);
		m->senLineCV = CreateMV("senLineCV2", 10, size);
		m->limboLineCV = CreateMV("limboLineCV2", 12, size);
		m->clerkLock = CreateMV("clerkLock2", 10, size);
		m->clerkCV = CreateMV("clerkCV2", 8, size);
		m->breakCV = CreateCondition("breakCV2", 8);
		m->lineCount = CreateMV("lineCount2", 10, size);
		m->bribeLineCount = CreateMV("bribeLineCount2", 15, size);
		m->senLineCount = CreateMV("senLineCount2", 13, size);
		m->clerkState = CreateMV("clerkState2", 11, size);
		m->numOfClerks = CreateMV("numOfClerks2", 12, 1);
		m->cashReceived = CreateMV("cashReceived2", 13, 1);
		m->currentCustomer = CreateMV("currentCustomer2", 16, size);
		m->numCustomersInLimbo = CreateMV("numCustomersInLimbo2", 20, 1);
		lockName = "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC";
	} else if(clerkType_ == 3) {
		m->lineLock = CreateLock("lineLock3", 9);
		m->newClerkIdLock = CreateLock("newClerkIdLock3", 15);
		m->newClerkId = CreateMV("newClerkId3", 11, 1);
		m->clerkType = CreateMV("clerkType3", 10, 1);
		m->lineCV = CreateMV("lineCV3", 7, size);
		m->bribeLineCV = CreateMV("bribeLineCV3", 12, size);
		m->senLineCV = CreateMV("senLineCV3", 10, size);
		m->limboLineCV = CreateMV("limboLineCV3", 12, size);
		m->clerkLock = CreateMV("clerkLock3", 10, size);
		m->clerkCV = CreateMV("clerkCV3", 8, size);
		m->breakCV = CreateCondition("breakCV3", 8);
		m->lineCount = CreateMV("lineCount3", 10, size);
		m->bribeLineCount = CreateMV("bribeLineCount3", 15, size);
		m->senLineCount = CreateMV("senLineCount3", 13, size);
		m->clerkState = CreateMV("clerkState3", 11, size);
		m->numOfClerks = CreateMV("numOfClerks3", 12, 1);
		m->cashReceived = CreateMV("cashReceived3", 13, 1);
		m->currentCustomer = CreateMV("currentCustomer3", 16, size);
		m->numCustomersInLimbo = CreateMV("numCustomersInLimbo3", 20, 1);
		lockName = "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD";
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
		SetMV(m->clerkLock, i, CreateLock(lockName, nameLength));
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
		/*SetMV(numCustomersLeft, 0, size);*/

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
