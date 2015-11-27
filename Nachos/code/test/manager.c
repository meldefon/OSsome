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

void checkForClerkOnBreak(struct Monitor *clerk) {

	int clerksOnBreak;
	int i;
	int j;	
	int k;
	int lineThreshold;
	int senLineThreshold;
	int numOfClerks;
	int clerkType;
	int senWorking;
	int lineCount;
	int senLineCount;
	int bribeLineCount;
	int clerkState;

	Acquire(clerk->lineLock);

	clerksOnBreak = 0;
	clerkType = GetMV(clerk->clerkType, 0);

	/*check if there are any clerks on break*/
	numOfClerks = GetMV(clerk->numOfClerks, 0);

	for(i = 0; i <numOfClerks; i++) {
		clerkState = GetMV(clerk->clerkState, i);
		if(clerkState == 1) {
			clerksOnBreak = 1;
			break;
		}
	}

	lineThreshold = 0;
	senLineThreshold = 0;
	if(clerksOnBreak == 1) {
		/*check if there is a particular line with more than 3 customers waiting*/
		for(k = 0; k < numOfClerks; k++) {
			senLineCount = GetMV(clerk->senLineCount, 0);
			lineCount = GetMV(clerk->lineCount, k);
			bribeLineCount = GetMV(clerk->bribeLineCount, k);
			senWorking = GetMV(senatorWorking, 0);

			if(senLineCount > senLineThreshold || (senWorking==NULL && (lineCount > lineThreshold ||
					bribeLineCount > lineThreshold))) {
			
			for(j = 0; j < numOfClerks; j++) {
					clerkState = GetMV(clerk->clerkState, j);
					if(clerkState == 1) { /*if a clerk is on break, wake them up*/
						Signal(clerk->breakCV, clerk->lineLock);

						if(clerkType == 0)
							Uprintf("Manager has woken up an ApplicationClerk.\n", 42, 0, 0, 0, 0);
						else {
							if(clerkType == 1) {
								Uprintf("Manager has woken up a PictureClerk.\n", 38, 0, 0, 0, 0);
							} else if(clerkType == 3) {
								Uprintf("Manager has woken up a Cashier.\n", 33, 0, 0, 0, 0);								
							} else if(clerkType == 2) {
								Uprintf("Manager has woken up a PassportClerk.\n", 39, 0, 0, 0, 0);
							}								
						}
					}
				}
				break;
			}
		}
	}
	Release(clerk->lineLock);
}

int main() {

	int myID;
	int i;
	int customersLeft;
	int appClerkCash;
	int picClerkCash;
	int passPClerkCash;
	int cashierCash;
	int totalCash;

	createServerMVs(1,1);
	initialize(&picClerk, 1, 1);
	initialize(&appClerk, 0, 1);
	initialize(&passPClerk, 2, 1);
	initialize(&cashier, 3, 1);

	myID = 0;	
	customersLeft = GetMV(numCustomersLeft, 0);
	while (customersLeft>0) {		
		checkForClerkOnBreak(&appClerk);
		checkForClerkOnBreak(&picClerk);
		checkForClerkOnBreak(&passPClerk);
		checkForClerkOnBreak(&cashier);

		appClerkCash = GetMV(appClerk.cashReceived, 0);
		picClerkCash = GetMV(picClerk.cashReceived, 0);
		passPClerkCash = GetMV(passPClerk.cashReceived, 0);
		cashierCash = GetMV(cashier.cashReceived, 0);
		totalCash = appClerkCash + picClerkCash + passPClerkCash + cashierCash;

		Uprintf("Manager has counted a total of $%d for ApplicationClerks.\n", 58, appClerkCash, 0,0,0);
		Uprintf("Manager has counted a total of $%d for PictureClerks.\n", 54, picClerkCash, 0,0,0);
		Uprintf("Manager has counted a total of $%d for PassportClerks.\n", 55, passPClerkCash, 0,0,0);
		Uprintf("Manager has counted a total of $%d for Cashiers.\n", 49, cashierCash, 0,0,0);
		Uprintf("Manager has counted a total of $%d for the passport office.\n", 60, totalCash, 0,0,0);
		
		/*go on "break" per say by only checking periodically*/
		for(i = 0; i < 20; i++) {
			Yield();
		}

		customersLeft = GetMV(numCustomersLeft, 0);
	}

	Exit(0);
	return 0;
}












