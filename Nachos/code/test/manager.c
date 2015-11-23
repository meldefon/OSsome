#include "syscall.h"
#include "globalVars.h"
#define NULL 0

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












