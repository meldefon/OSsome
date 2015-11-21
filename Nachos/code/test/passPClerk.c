#include "syscall.h"
#include "globalVars.h"
#define NULL 0

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