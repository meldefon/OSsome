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
	int ifPassportClerkChecked;
	int ifCashierChecked;
		
	createServerMVs(1,1);
	initialize(&picClerk, 1, 1);
	initialize(&appClerk, 0, 1);
	initialize(&passPClerk, 2, 1);
	initialize(&cashier, 3, 1);

	Acquire(cashier.newClerkIdLock);
	/*
	myLineID = cashier.newClerkId;
	id = cashier.newClerkId;
	cashier.newClerkId++;
	*/
	myLineID = GetMV(cashier.newClerkId, 0);
	id = myLineID;
	SetMV(cashier.newClerkId, 0, id + 1);
	Release(cashier.newClerkIdLock);	

	firstTime = 1;
	start = 1;

	while (start == 1) {		
		waitForLine(&cashier, id, firstTime);

		/*
		workLock = cashier.clerkLock[myLineID];
		workCV = cashier.clerkCV[myLineID];
		*/

		workLock = GetMV(cashier.clerkLock, myLineID);
		workCV = GetMV(cashier.clerkCV, myLineID);

		/*Now the clerk has been woken up and has been told the customer ID
		  Check*/
		/* customerSSN = cashierCurrentCustomer[myLineID];*/
		customerSSN = GetMV(cashierCurrentCustomer, myLineID);
		
		Uprintf("Cashier #%d has received SSN %d from Customer #%d.\n", 51, id, customerSSN,customerSSN,0);
		
		/* cashierChecked[customerSSN] = passportClerkChecked[customerSSN]; */

		ifPassportClerkChecked = GetMV(passportClerkChecked, customerSSN);
		SetMV(cashierChecked, customerSSN, ifPassportClerkChecked);

		Signal(workCV, workLock);
		Wait(workCV, workLock);

		ifCashierChecked = GetMV(cashierChecked, customerSSN);

		if (ifCashierChecked == 0) {
			/*Now customer is gone*/
			Uprintf("Cashier #%d has received $100 from Customer #%d before certification. They are to go to the back of the line\n", 109, id, customerSSN,0,0);
			Release(workLock);
			continue;
			/*THIS IS THE LINE THAT CUASED THE HUGE BUG, IN PROJECT 2 BUT NOT PROJECT 1 - no exit
			return;*/
		}

		/*Now customer has paid, so give passport and mark*/
		Uprintf("Cashier #%d has verified that Customer #%d has been certified by a PassportClerk\n", 81, id, customerSSN,0,0);
		Uprintf("Cashier #%d has recieved $100 from Customer #%d after certification\n", 68, id, customerSSN,0,0);
		Uprintf("Cashier #%d provided Customer #%d with their completed passport\n", 68, id, customerSSN,0,0);		
		
		/*
		cashier.cashReceived+=100;
		gottenPassport[customerSSN] = 1;
		*/

		cash = GetMV(cashier.cashReceived, 0) + 100;
		SetMV(cashier.cashReceived, 0, cash);
		SetMV(gottenPassport, customerSSN, 1);
		
		Signal(workCV, workLock);
		Wait(workCV, workLock);

		/*Now customer has left*/
		Uprintf("Cashier #%d has recorded that Customer #%d has been given their completed passport\n", 83, id, customerSSN,0,0);
		firstTime = 0;
		Release(workLock);
	}
	return 0;
}