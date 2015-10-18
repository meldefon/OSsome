#include "globalVars.h"
#define NULL 0

int waitForLine(typedef struct Monitor *clerk,int myLineID, int firstTime){
	int ifBribed;
	int start;
	Acquire(clerk->lineLock);
	ifBribed = 0;
	start = 1;
	while(start == 1) {

		if(senatorWorking!=NULL){

			if( clerk->senLineCount[myLineID]>0) {
				Signal(clerk->senLineCV[myLineID], clerk->lineLock);
				clerk->clerkState[myLineID] = 0;
				break;
			} else {
				clerk->clerkState[myLineID] = 1; /*set state to break*/
				Wait(clerk->breakCV, clerk->lineLock);
				continue;
			}
		}
		/*if there is someone in the bribe line, signal them first*/
		else if (clerk->bribeLineCount[myLineID] > 0) {
			Signal(clerk->bribeLineCV[myLineID], clerk->lineLock);
			ifBribed = 1;
			clerk->clerkState[myLineID] = 0; /*set state to busy*/
			if(strcmp(clerk->clerkType, "ApplicationClerk") == 0) {
				Uprintf("ApplicationClerk #%d has signalled a Customer to come to their counter.\n", 72, myLineID, 0, 0, 0);
			} else if(strcmp(clerk->clerkType, "PictureClerk") == 0) {
				Uprintf("PictureClerk #%d has signalled a Customer to come to their counter.\n", 68, myLineID, 0, 0, 0);
			} else if(strcmp(clerk->clerkType, "Cashier") == 0) {
				Uprintf("Cashier #%d has signalled a Customer to come to their counter.\n", 63, myLineID, 0, 0, 0);
			} else if(strcmp(clerk->clerkType, "PassportClerk") == 0) {
				Uprintf("PassportClerk #%d has signalled a Customer to come to their counter.\n", 69, myLineID, 0, 0, 0);
			}
			break;
		} else if (clerk->lineCount[myLineID] > 0) { /*signal someone in normal line*/
			Signal(clerk->lineCV[myLineID], clerk->lineLock);
			clerk->clerkState[myLineID] = 0; /*set state to busy*/
			if(strcmp(clerk->clerkType, "ApplicationClerk") == 0) {
				Uprintf("ApplicationClerk #%d has signalled a Customer to come to their counter.\n", 72, myLineID, 0, 0, 0);
			} else if(strcmp(clerk->clerkType, "PictureClerk") == 0) {
				Uprintf("PictureClerk #%d has signalled a Customer to come to their counter.\n", 68, myLineID, 0, 0, 0);
			} else if(strcmp(clerk->clerkType, "Cashier") == 0) {
				Uprintf("Cashier #%d has signalled a Customer to come to their counter.\n", 63, myLineID, 0, 0, 0);
			} else if(strcmp(clerk->clerkType, "PassportClerk") == 0) {
				Uprintf("PassportClerk #%d has signalled a Customer to come to their counter.\n", 69, myLineID, 0, 0, 0);
			}
			break;
		} else { /*no one is in either line, we must go to sleep*/

			firstTime = 0;
			if (firstTime == 0) {
			
				if(strcmp(clerk->clerkType, "ApplicationClerk") == 0) {
					Uprintf("ApplicationClerk #%d is going on break.\n", 40, myLineID, 0, 0, 0);
				} else if(strcmp(clerk->clerkType, "PictureClerk") == 0) {
					Uprintf("PictureClerk #%d is going on break.\n", 36, myLineID, 0, 0, 0);
				} else if(strcmp(clerk->clerkType, "Cashier") == 0) {
					Uprintf("Cashier #%d is going on break.\n", 31, myLineID, 0, 0, 0);
				} else if(strcmp(clerk->clerkType, "PassportClerk") == 0) {
					Uprintf("PassportClerk #%d is going on break.\n", 37, myLineID, 0, 0, 0);
				}				
				clerk->clerkState[myLineID] = 1; /*set state to break*/
				Wait(clerk->breakCV, clerk->lineLock);
				
				if(strcmp(clerk->clerkType, "ApplicationClerk") == 0) {
					Uprintf("ApplicationClerk #%d is coming on break.\n", 41, myLineID, 0, 0, 0);
				} else if(strcmp(clerk->clerkType, "PictureClerk") == 0) {
					Uprintf("PictureClerk #%d is going on break.\n", 37, myLineID, 0, 0, 0);
				} else if(strcmp(clerk->clerkType, "Cashier") == 0) {
					Uprintf("Cashier #%d is going on break.\n", 32, myLineID, 0, 0, 0);
				} else if(strcmp(clerk->clerkType, "PassportClerk") == 0) {
					Uprintf("PassportClerk #%d is going on break.\n", 38, myLineID, 0, 0, 0);
				}			
			}

			clerk->clerkState[myLineID] = 0; /*set state to busy*/
			continue;
		}
	}

	/*grab the clerkLock so we can properly signal the waiting customer
	to avoid a race condition and guarentee the correct order of events*/
	Acquire(clerk->clerkLock[myLineID]);

	/*now we can let go of line lock since we properly acquired the clerk lock*/
	Release(clerk->lineLock);
	Wait(clerk->clerkCV[myLineID], clerk->clerkLock[myLineID]);
	return ifBribed;
}

void pictureClerk(int id) {
	
	int myLineID;
	int firstTime;
	int ifBribed;
	int picClerkCV;
	int picClerkLock;
	int yieldCalls;
	int i;
	int start;

	start = 1;
	myLineID = id; /*the index we pass in will be used as id's for the clerks*/
	firstTime = 1;
	while(start == 1) {	
		ifBribed = waitForLine(&picClerk, id, firstTime);

		Uprintf("PictureClerk #%d has received SSN %d from Customer #%d.\n", 56, id, picClerk.currentCustomer[id], picClerk.currentCustomer[id],0);
		Uprintf("PictureClerk #%d has taken a picture of Customer #%d.\n", 54, id, picClerk.currentCustomer[id],0,0);

		picClerkCV = picClerk.clerkCV[myLineID];
		picClerkLock = picClerk.clerkLock[myLineID];
		
		Signal(picClerkCV, picClerkLock);
		Wait(picClerkCV, picClerkLock);

		if(customersWithCompletedPics[picClerk.currentCustomer[id]] == 1) {
			Uprintf("PictureClerk #%d has been told that Customer #%d does like their picture.\n", 74, id, picClerk.currentCustomer[id],0,0);
			yieldCalls = Rand(80,21);
			for(i = 0; i < yieldCalls; i++) { /*delay in filing the picture*/
				Yield();
			}

		} else {
			Uprintf("PictureClerk #%d has been told that Customer #%d does not like their picture.\n", 78, id, picClerk.currentCustomer[id],0,0);
		}

		if(ifBribed == 1) {
			Uprintf("PictureClerk #%d has received $500 from Customer #%d.\n", 54, id, picClerk.currentCustomer[id],0,0);
			picClerk.cashReceived+=500;
		}

		firstTime = 0;
		Release(picClerkLock);
	}
}

void applicationClerk(int id) {
	
	int myLineID;
	int firstTime;
	int ifBribed;
	int appClerkCV;
	int appClerkLock;
	int yieldCalls;
	int i;	
	int start;

	myLineID = id; /*the index we pass in will be used as id's for the clerks*/
	firstTime = 1;
	start = 1;

	while(start == 1) {	
		waitForLine(&appClerk, id, firstTime);

		Uprintf("ApplicationClerk #%d has received SSN %d from Customer #%d.\n", 60, id, appClerk.currentCustomer[id],appClerk.currentCustomer[id],0);
		
		appClerkCV = appClerk.clerkCV[myLineID];
		appClerkLock = appClerk.clerkLock[myLineID];

		/*input the socialSecurityNum into the completed applications*/
		customersWithCompletedApps[appClerk.currentCustomer[id]] = 1;

		yieldCalls = Rand(80, 21);
		for(i = 0; i < yieldCalls; i++) { /*delay in filing the application*/
			Yield();
		}

		Signal(appClerkCV, appClerkLock);
		Wait(appClerkCV, appClerkLock);

		Uprintf("ApplicationClerk #%d has recorded a completed application for Customer #%d.\n", 76, id, appClerk.currentCustomer[id],0,0);
		
		if(ifBribed == 1) {
			Uprintf("ApplicationClerk #%d has received $500 from Customer #%d.\n", 58, id, appClerk.currentCustomer[id],0,0);			
			appClerk.cashReceived+=500;
		}

		firstTime = 0;
		Release(appClerkLock);
	}
}

void passportClerk(int id) {

	int myLineID;
	int firstTime;
	int ifBribed;
	int workLock;
	int workCV;
	int customerSSN;
	int start;

	myLineID = id;
	firstTime = 1;
	start = 1;

	while(start == 1) {
		ifBribed = waitForLine(&passPClerk, id, firstTime);

		workLock = passPClerk.clerkLock[myLineID];
		workCV = passPClerk.clerkCV[myLineID];

		/*Now the clerk has been woken up and has been told the customer ID
		  Check*/
		customerSSN = passportClerkCurrentCustomer[myLineID];

		Uprintf("PassportClerk #%d has received SSN %d from Customer #%d.\n", 57, id, customerSSN,customerSSN,0);
		
		if(customersWithCompletedApps[customerSSN] == 1 && customersWithCompletedPics[customerSSN] == 1)
			passportClerkChecked[customerSSN] = 1;
		else
			passportClerkChecked[customerSSN] = 0;

		if(passportClerkChecked[customerSSN] == 0){
			Uprintf("PassportClerk #%d has determined that Customer #%d does not have both their application and picture completed\n", 110, id, customerSSN,0,0);
		}
		else {
			Uprintf("PassportClerk #%d has determined that Customer #%d has both their application and picture completed\n", 100, id, customerSSN,0,0);
		}

		Signal(workCV, workLock);
		Wait(workCV, workLock);

		/*Now customer is gone*/
		if(passportClerkChecked[customerSSN] == 1) {
			Uprintf("PassportClerk #%d has recorded Customer #%d passport documentation\n", 67, id, customerSSN,0,0);
		}

		if(ifBribed == 1) {
			Uprintf("PassportClerk #%d has received $500 from Customer #%d.\n", 55, id, customerSSN,0,0);
			passPClerk.cashReceived+=500;
		}

		firstTime = 0;
		Release(workLock);
	}
}

void cashierDo(int id) {

	int myLineID;
	int firstTime;
	int ifBribed;
	int workLock;
	int workCV;
	int customerSSN;
	int start; 

	myLineID = id;
	firstTime = 1;
	start = 1;

	while (start == 1) {
		waitForLine(&cashier, id, firstTime);

		workLock = cashier.clerkLock[myLineID];
		workCV = cashier.clerkCV[myLineID];

		/*Now the clerk has been woken up and has been told the customer ID
		  Check*/
		customerSSN = cashierCurrentCustomer[myLineID];
		
		Uprintf("Cashier #%d has received SSN %d from Customer #%d.\n", 51, id, customerSSN,customerSSN,0);

		cashierChecked[customerSSN] = passportClerkChecked[customerSSN];
		Signal(workCV, workLock);
		Wait(workCV, workLock);

		if (cashierChecked[customerSSN] == 0) {
			/*Now customer is gone*/
			Uprintf("Cashier #%d has received $100 from Customer #%d before certification. They are to go to the back of the line\n", 109, id, customerSSN,0,0);
			Release(workLock);
			return;
		}

		/*Now customer has paid, so give passport and mark*/
		Uprintf("Cashier #%d has verified that Customer #%d has been certified by a PassportClerk\n", 81, id, customerSSN,0,0);
		Uprintf("Cashier #%d has recieved $100 from Customer #%d after certification\n", 68, id, customerSSN,0,0);
		Uprintf("Cashier #%d provided Customer #%d with their completed passport\n", 68, id, customerSSN,0,0);		
		
		cashier.cashReceived+=100;
		gottenPassport[customerSSN] = 1;

		Signal(workCV, workLock);
		Wait(workCV, workLock);

		/*Now customer has left*/
		Uprintf("Cashier #%d has recorded that Customer #%d has been given their completed passport\n", 83, id, customerSSN,0,0);
		firstTime = 0;
		Release(workLock);
	}
}

void checkForClerkOnBreak(typedef struct Monitor *clerk) {

	int clerksOnBreak;
	int i;
	int j;	
	int k;
	int lineThreshold;
	int senLineThreshold;
	Acquire(clerk->lineLock);

	clerksOnBreak = 0;

	/*check if there are any clerks on break*/

	for(i = 0; i <clerk->numOfClerks; i++) {
		if(clerk->clerkState[i] == 1) {
			clerksOnBreak = 1;
			break;
		}
	}

	lineThreshold = 0;
	senLineThreshold = 0;
	if(clerksOnBreak == 1) {
		/*check if there is a particular line with more than 3 customers waiting*/
		for(k = 0; k < clerk->numOfClerks; k++) {
			if(clerk->senLineCount[0] > senLineThreshold || (senatorWorking==NULL && (clerk->lineCount[k] > lineThreshold ||
					clerk->bribeLineCount[k] > lineThreshold))) {
			
			for(j = 0; j < clerk->numOfClerks; j++) {
					if(clerk->clerkState[j] == 1) { /*if a clerk is on break, wake them up*/
						Signal(clerk->breakCV, clerk->lineLock);

						if(strcmp(clerk->clerkType, "ApplicationClerk") == 0)
							Uprintf("Manager has woken up an ApplicationClerk.\n", 42, 0, 0, 0, 0);
						else {
							if(strcmp(clerk->clerkType, "PictureClerk") == 0) {
								Uprintf("Manager has woken up a PictureClerk.\n", 38, 0, 0, 0, 0);
							} else if(strcmp(clerk->clerkType, "Cashier") == 0) {
								Uprintf("Manager has woken up a Cashier.\n", 33, 0, 0, 0, 0);								
							} else if(strcmp(clerk->clerkType, "PassportClerk") == 0) {
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

void managerDo(int id) {

	int myID;
	int i;
	myID = id;

	while (numCustomersLeft>0) {
		checkForClerkOnBreak(&appClerk);
		checkForClerkOnBreak(&picClerk);
		checkForClerkOnBreak(&passPClerk);
		checkForClerkOnBreak(&cashier);

		Uprintf("Manager has counted a total of $%d for ApplicationClerks.\n", 58, appClerk.cashReceived, 0,0,0);
		Uprintf("Manager has counted a total of $%d for PictureClerks.\n", 54, picClerk.cashReceived, 0,0,0);
		Uprintf("Manager has counted a total of $%d for PassportClerks.\n", 55, passPClerk.cashReceived, 0,0,0);
		Uprintf("Manager has counted a total of $%d for Cashiers.\n", 49, cashier.cashReceived, 0,0,0);
		Uprintf("Manager has counted a total of $%d for the passport office.\n", 60, appClerk.cashReceived + picClerk.cashReceived + passPClerk.cashReceived + cashier.cashReceived, 0,0,0);
		
		/*go on "break" per say by only checking periodically*/
		for(i = 0; i < 20; i++) {
			Yield();
		}
	}	
}












