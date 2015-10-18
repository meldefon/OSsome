#include "globalVars.h"

void Uprintf(char* string, int length, int num_1, int num_2, int num_3, int num_4) {
    Printf(string, length, (num_1 * 100000) + num_2, (num_3 * 100000) + num_4);
}

int punishTime = 100;

Monitor appClerk, picClerk, passPClerk, cashier;

/*global shared data between the clerks that are used for filing purposes */
int customersWithCompletedApps[50];
int customersWithCompletedPics[50];
int passportClerkChecked[50];
int cashierChecked[50];
int gottenPassport[50];
int cashReceived[50];
int bribesEnabled;

int appClerkCurrentCustomer[50];
int pictureClerkCurrentCustomer[50];
int passportClerkCurrentCustomer[50];
int cashierCurrentCustomer[50];

int senatorLock;
int senatorCV;
int isSenator[50];
int senatorWorking;
int clerksCanWork;

int numCustomersLeft;

void punish(int time){
	for (int i = 0; i < time; i++) {
		currentThread->Yield();
	}
}

void tellPassportClerkSSN(int SSN,int myLine) {
	passportClerkCurrentCustomer[myLine] = SSN;
}

void tellCashierSSN(int SSN, int myLine) {
	cashierCurrentCustomer[myLine] = SSN;
}

void payCashier(int SSN, int *cash){
	cashReceived[SSN]+=100;
	cash-=100;
}


int getInLine(Monitor *clerk, int socialSecurityNum, int* cash) {

	if(senatorWorking!=NULL && senatorWorking==socialSecurityNum) {
		Acquire(clerk->lineLock);
		clerk->senLineCount[0]++;
		Wait(clerk->senLineCV[0], clerk->lineLock);
		clerk->senLineCount[0]--;
		Release(clerk->lineLock);
		return 0;
	}

		int* lineCount;
		int* lineCV;
		int didBribe = 0;

		int wantToBribe = Rand_syscall(10, 0); /* random choice about whether to bribe */

		if(strcmp(clerk->clerkType,"Cashier")){
			wantToBribe = 1;
		}

		if(wantToBribe==0 && *cash>100 && bribesEnabled == 1) {
			*cash-=500;
			lineCount = clerk->bribeLineCount;
			lineCV = clerk->bribeLineCV;
			didBribe = 1;
		} else {
			lineCount = clerk->lineCount;
			lineCV = clerk->lineCV;
		}


		/*possible clerk states are:
		0 = busy
		1 = break
		2 = avaiable
		clerk->lineLock->Acquire(); */

		/*grab the lock since we are dealing with shared data for the lines and clerks*/
		Acquire(clerk->lineLock);

		/*pick the shortest clerk line*/
		int myLine = -1;

		while(myLine == -1) { /*if we haven't found a line, we need to loop back again*/

			int lineSize = 777;

			for(int i = 0; i < clerk->numOfClerks; i++) {
				if(clerk->clerkState[i] == 2) { /*or the clerk is available*/
					myLine = i;
					lineSize = lineCount[i];
					break; /*leave since we found the right clerk*/
				}
				/*Right now, customer picks shortest line regardless of break status*/
				else if(lineCount[i] < lineSize) { /*if the line we are in is the shortest and the clerk is not on break, consider that line*/
					myLine = i;
					lineSize = lineCount[i];
				}
			}
			
			/*if all the clerks on are on break*/
			if(myLine == -1) {
				if(strcmp(clerk->clerkType, "ApplicationClerk") == 0) {
					Uprintf("Customer #%d has entered ApplicationClerk Limbo Line.\n", 54, socialSecurityNum, 0, 0, 0);
				} else if(strcmp(clerk->clerkType, "PictureClerk") == 0) {
					Uprintf("Customer #%d has entered PictureClerk Limbo Line.\n", 50, socialSecurityNum, 0, 0, 0);
				} else if(strcmp(clerk->clerkType, "Cashier") == 0) {
					Uprintf("Customer #%d has entered Cashier Limbo Line.\n", 45, socialSecurityNum, 0, 0, 0);
				} else if(strcmp(clerk->clerkType, "PassportClerk") == 0) {
					Uprintf("Customer #%d has entered PassportClerk Limbo Line.\n", 51, socialSecurityNum, 0, 0, 0);
				}	
				clerk->numCustomersInLimbo++;
				Wait(clerk->limboLineCV, clerk->lineLock);
			}
		}
		
		if(clerk->clerkState[myLine] == 0 || clerk->clerkState[myLine]==1) { /*if the clerk is busy with another customer, we must wait, else just bypass this and go straight to transaction*/
			if(didBribe == 0) {
				if(strcmp(clerk->clerkType, "ApplicationClerk") == 0) {
					Uprintf("Customer #%d has gotten in regular line for ApplicationClerk #%d.\n", 66, socialSecurityNum, myLine, 0, 0);
				} else if(strcmp(clerk->clerkType, "PictureClerk") == 0) {
					Uprintf("Customer #%d has gotten in regular line for PictureClerk #%d.\n", 62, socialSecurityNum, myLine, 0, 0);
				} else if(strcmp(clerk->clerkType, "Cashier") == 0) {
					Uprintf("Customer #%d has gotten in regular line for Cashier #%d.\n", 57, socialSecurityNum, myLine, 0, 0);
				} else if(strcmp(clerk->clerkType, "PassportClerk") == 0) {
					Uprintf("Customer #%d has gotten in regular line for PassportClerk #%d.\n", 63, socialSecurityNum, myLine, 0, 0);
				}							
			} else {
				if(strcmp(clerk->clerkType, "ApplicationClerk") == 0) {
					Uprintf("Customer #%d has gotten in bribe line for ApplicationClerk #%d.\n", 64, socialSecurityNum, myLine, 0, 0);
				} else if(strcmp(clerk->clerkType, "PictureClerk") == 0) {
					Uprintf("Customer #%d has gotten in bribe line for PictureClerk #%d.\n", 60, socialSecurityNum, myLine, 0, 0);
				} else if(strcmp(clerk->clerkType, "Cashier") == 0) {
					Uprintf("Customer #%d has gotten in bribe line for Cashier #%d.\n", 55, socialSecurityNum, myLine, 0, 0);
				} else if(strcmp(clerk->clerkType, "PassportClerk") == 0) {
					Uprintf("Customer #%d has gotten in bribe line for PassportClerk #%d.\n", 61, socialSecurityNum, myLine, 0, 0);
				}					
			}

			lineCount[myLine]++; /*get in line*/
			Wait(lineCV[myLine], clerk->lineLock);
			lineCount[myLine]--; /*get out of line and move to the counter*/
		}

			
		clerk->clerkState[myLine] = 0;   /*set the clerk status to busy*/
		Release(clerk->lineLock);
		return myLine;
}

void doAppClerkStuff(int socialSecurityNum, int* cash) {

	int myLine = getInLine(&appClerk,socialSecurityNum, cash);

	/*now we must obtain the lock from the AppClerk which went to wait state once he was avaiable and 
	waiting for a customer to signal him*/
	
	Acquire(appClerk.clerkLock[myLine]);	
	appClerk.currentCustomer[myLine] = socialSecurityNum; /*which customer is currently being served*/
	Uprintf("Customer #%d has given SSN %d to ApplicationClerk #%d.\n", 55, socialSecurityNum, socialSecurityNum, myLine, 0);

	Signal(appClerk.clerkCV[myLine], appClerk.clerkLock[myLine]);
	Wait(appClerk.clerkCV[myLine], appClerk.clerkLock[myLine]);
	Signal(appClerk.clerkCV[myLine], appClerk.clerkLock[myLine]);
	Release(appClerk.clerkLock[myLine]); /*let go of the lock*/
}

int doPicClerkStuff(int socialSecurityNum, int* cash) {

	int myLine = getInLine(&picClerk,socialSecurityNum,cash);
	int choice = 0;

	/*now we must obtain the lock from the PicClerk which went to wait state once he was avaiable and 
	waiting for a customer to signal him*/
	Acquire(picClerk.clerkLock[myLine]);
	picClerk.currentCustomer[myLine] = socialSecurityNum; /*which customer is currently being served*/

	Uprintf("Customer #%d has given SSN %d to PictureClerk #%d.\n", 51, socialSecurityNum, socialSecurityNum, myLine, 0);
	Signal(picClerk.clerkCV[myLine], picClerk.clerkLock[myLine]);
	/*wait for the clerk to confirm then we decide if we like the photo or not*/
	Wait(picClerk.clerkCV[myLine], picClerk.clerkLock[myLine]);	
	int probablity = Rand_syscall(10, 0); /*generate a random # from 0 - 9, if less than or equal to 4, we retake photo*/

	if(probablity <= 4) { /*if we disliked the photo*/
			Uprintf("Customer #%d does not like their picture from PictureClerk #%d.\n", 64, socialSecurityNum, myLine, 0, 0);
			customersWithCompletedPics[socialSecurityNum] = 0;
			choice = 1;
	} else {
			Uprintf("Customer #%d likes their picture from PictureClerk #%d.\n", 61, socialSecurityNum, myLine, 0, 0);
			customersWithCompletedPics[socialSecurityNum] = 1;
			choice = 0;		
	}

	Signal(picClerk.clerkCV[myLine], picClerk.clerkLock[myLine]);	
	Release(picClerk.clerkLock[myLine]);
	
	return choice;
}

void doPassportClerkStuff(int socialSecurityNum,int*cash){

	int mySSN = socialSecurityNum;

	/*First get in line with a generic method*/
	int myLine = getInLine(&passPClerk,socialSecurityNum,cash);

	int workLock = passPClerk.clerkLock[myLine];
	int workCV = passPClerk.clerkCV[myLine];
	
	Acquire(workLock);

	/*Tell Clerk CV, then wait*/
	Uprintf("Customer #%d has given SSN %d to PassportClerk #%d.\n", 52, socialSecurityNum, socialSecurityNum, myLine, 0);
	tellPassportClerkSSN(mySSN,myLine);
	Signal(workCV, workLock);
	Wait(workCV, workLock);

	/*Now leave*/
	Signal(workCV, workLock);
	Release(workLock);

	/*Decide whether to self-punish*/
	int myPassportChecked = passportClerkChecked[mySSN];
	
	if(myPassportChecked == 0) {
		Uprintf("Customer #%d has gone to PassportClerk #%d too soon. They are going to the back of the line.\n", 93, socialSecurityNum, myLine,0, 0);
		punish(punishTime);
	}
	return;
}

void doCashierStuff(int mySSN, int* cash){

	int socialSecurityNum = mySSN;
	
	/*First get in line with a generic method*/
	int myLine = getInLine(&cashier,socialSecurityNum,cash);

	int workLock = cashier.clerkLock[myLine];
	int workCV = cashier.clerkCV[myLine];
	Acquire(workLock);

	/*Tell Clerk CV, then wait*/
	Uprintf("Customer #%d has given SSN %d to Cashier #%d.\n", 47, socialSecurityNum, socialSecurityNum, myLine, 0);
	tellCashierSSN(mySSN,myLine);
	Signal(workCV, workLock);
	Wait(workCV, workLock);

	/*Decide weather to self-punish*/
	int readyToPay = cashierChecked[mySSN];	
	if(readyToPay == 0) {
		/*Release, punish, and leave*/
		Uprintf("Customer #%d has gone to Cashier #%d too soon. They are going to the back of the line.\n", 87, socialSecurityNum, myLine,0, 0);
		Signal(workCV, workLock);
		Release(workLock);
		punish(punishTime);
		return;
	}

	/*Now you can pay*/
	Uprintf("Customer #%d has given Cashier #%d $100\n", 41, socialSecurityNum, myLine,0, 0);
	payCashier(mySSN,cash);
	Signal(workCV, workLock);
	Wait(workCV, workLock);

	/*Now you've been woken up because you have the passport, so leave*/
	Signal(workCV, workLock);
	Release(workLock);
	return;
}

void senatorClearLines(){
	clerksCanWork = 0;
	Monitor allMonitors[4];
	allMonitors[0] = appClerk;
	allMonitors[1] = picClerk;
	allMonitors[2] = passPClerk;
	allMonitors[3] = cashier;

	/*Broadcast to all clerk lines so that custs wake up*/
	for(int i = 0;i<4;i++) {
		Acquire(allMonitors[i].lineLock);
		Broadcast(allMonitors[i].lineCV[0], allMonitors[i].lineLock);
		Release(allMonitors[i].lineLock);
	}
}

void customer(int social) {

	/*Customer variables*/
	int cash = 1100;
	int appClerkSeen = 0;
	int picClerkSeen = 0;
	int myLine;
	int socialSecurityNum;
	int picOrAppClerk;

	int canStartWorking = 1;
	if (isSenator[social] == 1) {
		canStartWorking = 0;
	}

	while(canStartWorking  == 0) {
		if (isSenator[social] == 1) {
			int senatorWaitTime = 3;

			Acquire(senatorLock);
			if (senatorWorking == NULL) {
				senatorWorking = social;
				canStartWorking = 1;
			}
			else {
				Wait(senatorCV, senatorLock);
			}
			Release(senatorLock);
		}
	}

	socialSecurityNum = social;
	picOrAppClerk = 0; /*0 for appClerk, 1 for picClerk, 2 for both completed*/
	
	int notCompleted = 1;

	while(notCompleted == 1) {

		/*Stop if there's a senator trying to work*/
		Acquire(senatorLock);
		if(senatorWorking!=NULL && senatorWorking!=social){
			Wait(senatorCV, senatorLock);
		}

		Release(senatorLock);

		/*ERROR CASE: pick the wrong behavior, go ahead and get punished*/
		int mistake = Rand_syscall(100,0);

		if(mistake==0){
			/*Which behavior will the customer pick*/
			int choice = Rand_syscall(2,0);

			if(choice==0){
				doPassportClerkStuff(socialSecurityNum,&cash);
				continue;
			}
			else{
				doCashierStuff(socialSecurityNum,&cash);
				continue;
			}
		}
		/*Do the picture/application clerk stuff*/
		if(customersWithCompletedApps[socialSecurityNum] == 0 || customersWithCompletedPics[socialSecurityNum] == 0) {
			/*enter if we choose to go to the appClerk*/
			if (picOrAppClerk == 0) {
				doAppClerkStuff(socialSecurityNum,&cash);
				picOrAppClerk = 1;
				continue;
			}

			/*enter if we choose to go to the picClerk*/
			if (picOrAppClerk == 1) {
				picOrAppClerk = doPicClerkStuff(socialSecurityNum,&cash);
				continue;
			}
		}
		/*Do the passportClerk stuff*/
		else if(passportClerkChecked[socialSecurityNum] == 0){
			doPassportClerkStuff(socialSecurityNum,&cash);
			continue;
		}
		/*Do the cashier stuff*/
		else if(cashierChecked[socialSecurityNum] == 0){
			doCashierStuff(socialSecurityNum,&cash);
			continue;
		}
		else{
			notCompleted = 0;
		}
	}

	/*Clean up, let people wake up*/
	if(isSenator[social] == 1){
		Uprintf("Customer (Senator)#%d is leaving the Passport Office.\n", 54, socialSecurityNum, 0, 0, 0);
		senatorWorking = NULL;
		Acquire(senatorLock);
		Broadcast(senatorCV, senatorLock);
		Release(senatorLock);
	}
	else {
		Uprintf("Customer #%d is leaving the Passport Office.\n", 45, socialSecurityNum, 0, 0, 0);
	}
	numCustomersLeft-=1;
	if(numCustomersLeft==0 && bribesEnabled == 0){
		int totalSales = appClerk.cashReceived + picClerk.cashReceived +
						 passPClerk.cashReceived + cashier.cashReceived;
		Uprintf("TOTAL SALES: %d\n", 16, totalSales, 0,0,0);
	}
}
