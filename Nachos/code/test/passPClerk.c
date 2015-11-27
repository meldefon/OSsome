#include "syscall.h"
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
		lockName = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\0";
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
		lockName = "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\0";
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
		lockName = "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\0";
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
		lockName = "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD\0";
	}

}

void Uprintf(char *string, int length, int num_1, int num_2, int num_3, int num_4) {
    Printf(string, length, (num_1 * 100000) + num_2, (num_3 * 100000) + num_4);
}

int waitForLine(struct Monitor *clerk, int myLineID, int firstTime) {
	
	int ifBribed;
	int senLineCount;
	int bribeLineCount;
	int lineCount;
	int senLineCV;
	int bribeLineCV;
	int lineCV;
	int start;
	int clerkLock;
	int clerkCV;
	int clerkType;
	int senWorking;

	Acquire(clerk->lineLock);
	
	ifBribed = 0;
	start = 1;
	clerkType = GetMV(clerk->clerkType, 0);

	while(start == 1) {
		
		bribeLineCount = GetMV(clerk->bribeLineCount, myLineID);
		lineCount = GetMV(clerk->lineCount, myLineID);
		senWorking = GetMV(senatorWorking, 0);

		if(senWorking != NULL){

			senLineCount = GetMV(clerk->senLineCount, myLineID);

			if(senLineCount > 0) {
				/*Signal(clerk->senLineCV[myLineID], clerk->lineLock);*/
				/*clerk->clerkState[myLineID] = 0;*/
				senLineCV = GetMV(clerk->senLineCV, myLineID);
				Signal(senLineCV, clerk->lineLock);
				SetMV(clerk->clerkState, myLineID, 0);
				break;
			} else {
				/*clerk->clerkState[myLineID] = 1;*/ /*set state to break*/
				SetMV(clerk->clerkState, myLineID, 1);
				Wait(clerk->breakCV, clerk->lineLock);
				continue;
			}
		} else if (bribeLineCount > 0) { /*if there is someone in the bribe line, signal them first*/
			/* Signal(clerk->bribeLineCV[myLineID], clerk->lineLock); */
			bribeLineCV = GetMV(clerk->bribeLineCV, myLineID);
			Signal(bribeLineCV, clerk->lineLock);
			ifBribed = 1;
			/* clerk->clerkState[myLineID] = 0; */ /*set state to busy*/
			SetMV(clerk->clerkState, myLineID, 0);

			if(clerkType == 0) {
				Uprintf("ApplicationClerk #%d has signalled a Customer to come to their counter.\n", 72, myLineID, 0, 0, 0);
			} else if(clerkType == 1) {
				Uprintf("PictureClerk #%d has signalled a Customer to come to their counter.\n", 68, myLineID, 0, 0, 0);
			} else if(clerkType == 3) {
				Uprintf("Cashier #%d has signalled a Customer to come to their counter.\n", 63, myLineID, 0, 0, 0);
			} else if(clerkType == 2) {
				Uprintf("PassportClerk #%d has signalled a Customer to come to their counter.\n", 69, myLineID, 0, 0, 0);
			}
			break;
		} else if (lineCount > 0) { /*signal someone in normal line*/
			/*Signal(clerk->lineCV[myLineID], clerk->lineLock);*/
			lineCV = GetMV(clerk->lineCV, myLineID);
			Signal(lineCV, clerk->lineLock);
			/*clerk->clerkState[myLineID] = 0; */ /*set state to busy*/
			SetMV(clerk->clerkState, myLineID, 0);
			
			if(clerkType == 0) {
				Uprintf("ApplicationClerk #%d has signalled a Customer to come to their counter.\n", 72, myLineID, 0, 0, 0);
			} else if(clerkType == 1) {
				Uprintf("PictureClerk #%d has signalled a Customer to come to their counter.\n", 68, myLineID, 0, 0, 0);
			} else if(clerkType == 3) {
				Uprintf("Cashier #%d has signalled a Customer to come to their counter.\n", 63, myLineID, 0, 0, 0);
			} else if(clerkType == 2) {
				Uprintf("PassportClerk #%d has signalled a Customer to come to their counter.\n", 69, myLineID, 0, 0, 0);
			}
			break;
		} else { /*no one is in either line, we must go to sleep*/

			firstTime = 0;
			if (firstTime == 0) {
				if(clerkType == 0) {
					Uprintf("ApplicationClerk #%d is going on break.\n", 40, myLineID, 0, 0, 0);
				} else if(clerkType == 1) {
					Uprintf("PictureClerk #%d is going on break.\n", 36, myLineID, 0, 0, 0);
				} else if(clerkType == 3) {
					Uprintf("Cashier #%d is going on break.\n", 31, myLineID, 0, 0, 0);
				} else if(clerkType == 2) {
					Uprintf("PassportClerk #%d is going on break.\n", 37, myLineID, 0, 0, 0);
				}				
				/*clerk->clerkState[myLineID] = 1;*/ /*set state to break*/
				SetMV(clerk->clerkState, myLineID, 1);

				Wait(clerk->breakCV, clerk->lineLock);
				
				if(clerkType == 0) {
					Uprintf("ApplicationClerk #%d is coming on break.\n", 41, myLineID, 0, 0, 0);
				} else if(clerkType == 1) {
					Uprintf("PictureClerk #%d is going on break.\n", 37, myLineID, 0, 0, 0);
				} else if(clerkType == 3) {
					Uprintf("Cashier #%d is going on break.\n", 32, myLineID, 0, 0, 0);
				} else if(clerkType == 2) {
					Uprintf("PassportClerk #%d is going on break.\n", 38, myLineID, 0, 0, 0);
				}			
			}

			/*clerk->clerkState[myLineID] = 0;*/ /*set state to busy*/
			SetMV(clerk->clerkState, myLineID, 0);
			continue;
		}
	}

	/*grab the clerkLock so we can properly signal the waiting customer
	to avoid a race condition and guarentee the correct order of events*/
	clerkLock = GetMV(clerk->clerkLock, myLineID);
	/*Acquire(clerk->clerkLock[myLineID]);*/
	Acquire(clerkLock);

	/*now we can let go of line lock since we properly acquired the clerk lock*/
	Release(clerk->lineLock);
	clerkCV = GetMV(clerk->clerkCV, myLineID);
	/*Wait(clerk->clerkCV[myLineID], clerk->clerkLock[myLineID]);*/
	Wait(clerkCV, clerkLock);
	return ifBribed;
}

int main() {
	
	int myLineID;
	int firstTime;
	int ifBribed;
	int workLock;
	int workCV;
	int customerSSN;
	int id;
	int start;
	int cash;
	int ifCompletedPic;
	int ifCompletedApp;	
	int ifPassportClerkChecked;

	start = 1;

	createServerMVs(1,1);
	initialize(&picClerk, 1, 1);
	initialize(&appClerk, 0, 1);
	initialize(&passPClerk, 2, 1);
	initialize(&cashier, 3, 1);

	Acquire(passPClerk.newClerkIdLock);
	/*
	myLineID = passPClerk.newClerkId;
	id = passPClerk.newClerkId;
	passPClerk.newClerkId++;
	*/
	myLineID = GetMV(passPClerk.newClerkId, 0);
	id = myLineID;
	SetMV(passPClerk.newClerkId, 0, id + 1);
	Release(passPClerk.newClerkIdLock);
	

	firstTime = 1;
	start = 1;

	while(start == 1) {
		ifBribed = waitForLine(&passPClerk, id, firstTime);

		/*
		workLock = passPClerk.clerkLock[myLineID];
		workCV = passPClerk.clerkCV[myLineID];
		*/

		workLock = GetMV(passPClerk.clerkLock, myLineID);
		workCV = GetMV(passPClerk.clerkCV, myLineID);

		/*Now the clerk has been woken up and has been told the customer ID
		  Check*/
		/* customerSSN = passportClerkCurrentCustomer[myLineID]; */

		customerSSN = GetMV(passportClerkCurrentCustomer, myLineID);

		Uprintf("PassportClerk #%d has received SSN %d from Customer #%d.\n", 57, id, customerSSN,customerSSN,0);
		
		ifCompletedApp = GetMV(customersWithCompletedApps, customerSSN);
		ifCompletedPic = GetMV(customersWithCompletedPics, customerSSN);

		if(ifCompletedApp == 1 && ifCompletedPic == 1) 
			SetMV(passportClerkChecked, customerSSN, 1); /*passportClerkChecked[customerSSN] = 1;*/
		else
			SetMV(passportClerkChecked, customerSSN, 0); /*passportClerkChecked[customerSSN] = 0;*/

		ifPassportClerkChecked = GetMV(passportClerkChecked, customerSSN);

		if(ifPassportClerkChecked == 0) {
			Uprintf("PassportClerk #%d has determined that Customer #%d does not have both their application and picture completed\n", 110, id, customerSSN,0,0);
		} else {
			Uprintf("PassportClerk #%d has determined that Customer #%d has both their application and picture completed\n", 100, id, customerSSN,0,0);
		}

		Signal(workCV, workLock);
		Wait(workCV, workLock);

		/*Now customer is gone*/
		if(ifPassportClerkChecked == 1) {
			Uprintf("PassportClerk #%d has recorded Customer #%d passport documentation\n", 67, id, customerSSN,0,0);
		}

		if(ifBribed == 1) {
			Uprintf("PassportClerk #%d has received $500 from Customer #%d.\n", 55, id, customerSSN,0,0);
			/*passPClerk.cashReceived+=500;*/
			cash = GetMV(passPClerk.cashReceived, 0) + 500;
			SetMV(passPClerk.cashReceived, 0, cash);
		}

		firstTime = 0;
		Release(workLock);
	}
	return 0;
}