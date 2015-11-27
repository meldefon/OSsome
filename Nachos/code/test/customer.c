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

int punishTime = 100;



void punish(int time){
	int i; 
	for (i = 0; i < time; i++) {
		Yield();
	}
}

void tellPassportClerkSSN(int SSN,int myLine) {
	/* passportClerkCurrentCustomer[myLine] = SSN; */
	SetMV(passportClerkCurrentCustomer, myLine, SSN);
}

void tellCashierSSN(int SSN, int myLine) {
	/* cashierCurrentCustomer[myLine] = SSN; */
	SetMV(cashierCurrentCustomer, myLine, SSN);
}

void payCashier(int SSN, int *cash){
	int newCash;
	newCash = GetMV(cashReceived, SSN) + 100;
	SetMV(cashReceived, SSN, newCash);
	/* cashReceived[SSN]+=100; */
	cash-=100;
}


int getInLine(struct Monitor *clerk, int socialSecurityNum, int *cash) {
	
	int lineCount;
	int lineCV;
	int didBribe;
	int wantToBribe;
	int myLine;
	int lineSize;
	int i;
	int senWorking;
	int senLineCount;
	int senLineCV; 
	int clerkType;
	int ifBribesEnabled;
	int numOfClerks;
	int clerkState;
	int numCustomersInLimbo;
	int size;
	
	senWorking = GetMV(senatorWorking, 0);
	clerkType = GetMV(clerk->clerkType, 0);

	if(senWorking!=NULL && senWorking==socialSecurityNum) {
		Acquire(clerk->lineLock);
		/* clerk->senLineCount[0]++; */
		senLineCount = GetMV(clerk->senLineCount, 0);
		SetMV(clerk->senLineCount, 0, senLineCount + 1);
		/*Wait(clerk->senLineCV[0], clerk->lineLock);*/
		senLineCV = GetMV(clerk->senLineCV, 0);
		Wait(senLineCV, clerk->lineLock);
		/* clerk->senLineCount[0]--; */
		senLineCount = GetMV(clerk->senLineCount, 0);
		SetMV(clerk->senLineCount, 0, senLineCount - 1);		
		Release(clerk->lineLock);
		return 0;
	}

	didBribe = 0;

	wantToBribe = Rand(10, 0); /* random choice about whether to bribe */

	if(clerkType == 3){
		wantToBribe = 1;
	}

	wantToBribe = 1;
	
	ifBribesEnabled = GetMV(bribesEnabled, 0);

	if(wantToBribe==0 && *cash>100 && ifBribesEnabled == 1) {
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
	myLine = -1;
	numOfClerks = GetMV(clerk->numOfClerks, 0);

	while(myLine == -1) { /*if we haven't found a line, we need to loop back again*/

		lineSize = 777;
		for(i = 0; i < numOfClerks; i++) {
			clerkState = GetMV(clerk->clerkState, i);
			if(clerkState == 2) { /*or the clerk is available*/
				myLine = i;
				/*lineSize = lineCount[i];*/
				lineSize = GetMV(lineCount, i);
				break; /*leave since we found the right clerk*/
			}
			/*Right now, customer picks shortest line regardless of break status*/
			else if(/*lineCount[i]*/ GetMV(lineCount, i) < lineSize) { /*if the line we are in is the shortest and the clerk is not on break, consider that line*/
				myLine = i;
				/*lineSize = lineCount[i];*/
				lineSize = GetMV(lineCount, i);
			}
		}
			
		/*if all the clerks on are on break*/
		if(myLine == -1) {
			if(clerkType == 0) {
				Uprintf("Customer #%d has entered ApplicationClerk Limbo Line.\n", 54, socialSecurityNum, 0, 0, 0);
			} else if(clerkType == 1) {
				Uprintf("Customer #%d has entered PictureClerk Limbo Line.\n", 50, socialSecurityNum, 0, 0, 0);
			} else if(clerkType == 3) {
				Uprintf("Customer #%d has entered Cashier Limbo Line.\n", 45, socialSecurityNum, 0, 0, 0);
			} else if(clerkType == 2) {
				Uprintf("Customer #%d has entered PassportClerk Limbo Line.\n", 51, socialSecurityNum, 0, 0, 0);
			}	
			/*clerk->numCustomersInLimbo++;*/
			numCustomersInLimbo = GetMV(clerk->numCustomersInLimbo, 0);
			SetMV(clerk->numCustomersInLimbo, 0, numCustomersInLimbo + 1);
			Wait(clerk->limboLineCV, clerk->lineLock);
		}
	}
	clerkState = GetMV(clerk->clerkState, myLine);

	if(clerkState == 0 || clerkState ==1) { /*if the clerk is busy with another customer, we must wait, else just bypass this and go straight to transaction*/
		if(didBribe == 0) {
			if(clerkType == 0) {
				Uprintf("Customer #%d has gotten in regular line for ApplicationClerk #%d.\n", 66, socialSecurityNum, myLine, 0, 0);
			} else if(clerkType == 1) {
				Uprintf("Customer #%d has gotten in regular line for PictureClerk #%d.\n", 62, socialSecurityNum, myLine, 0, 0);
			} else if(clerkType == 3) {
				Uprintf("Customer #%d has gotten in regular line for Cashier #%d.\n", 57, socialSecurityNum, myLine, 0, 0);
			} else if(clerkType == 2) {
				Uprintf("Customer #%d has gotten in regular line for PassportClerk #%d.\n", 63, socialSecurityNum, myLine, 0, 0);
			}							
		} else {
			if(clerkType == 0) {
				Uprintf("Customer #%d has gotten in bribe line for ApplicationClerk #%d.\n", 64, socialSecurityNum, myLine, 0, 0);
			} else if(clerkType == 1) {
				Uprintf("Customer #%d has gotten in bribe line for PictureClerk #%d.\n", 60, socialSecurityNum, myLine, 0, 0);
			} else if(clerkType == 3) {
				Uprintf("Customer #%d has gotten in bribe line for Cashier #%d.\n", 55, socialSecurityNum, myLine, 0, 0);
			} else if(clerkType == 2) {
				Uprintf("Customer #%d has gotten in bribe line for PassportClerk #%d.\n", 61, socialSecurityNum, myLine, 0, 0);
			}					
		}

		/* lineCount[myLine]++; */ /*get in line*/ 
		size = GetMV(lineCount, myLine);
		SetMV(lineCount, myLine, size + 1);
		/*Wait(lineCV[myLine], clerk->lineLock);*/
		Wait(GetMV(lineCV, myLine), clerk->lineLock);
		/*lineCount[myLine]--;*/ /*get out of line and move to the counter*/
		size = GetMV(lineCount, myLine);
		SetMV(lineCount, myLine, size - 1);
	}

			
	/*clerk->clerkState[myLine] = 0; */ /*set the clerk status to busy*/
	SetMV(clerk->clerkState, myLine, 0);
	Release(clerk->lineLock);
	return myLine;
}

void doAppClerkStuff(int socialSecurityNum, int *cash) {

	int myLine;
	int clerkCV;
	int clerkLock;

	myLine = getInLine(&appClerk,socialSecurityNum, cash);
	clerkCV = GetMV(appClerk.clerkCV, myLine);
	clerkLock = GetMV(appClerk.clerkLock, myLine);

	/*now we must obtain the lock from the AppClerk which went to wait state once he was avaiable and 
	waiting for a customer to signal him*/
	
	/*Acquire(appClerk.clerkLock[myLine]);*/
	Acquire(clerkLock);
	/*appClerk.currentCustomer[myLine] = socialSecurityNum;*/ /*which customer is currently being served*/
	SetMV(appClerk.currentCustomer, myLine, socialSecurityNum);
	Uprintf("Customer #%d has given SSN %d to ApplicationClerk #%d.\n", 55, socialSecurityNum, socialSecurityNum, myLine, 0);

	/*
	Signal(appClerk.clerkCV[myLine], appClerk.clerkLock[myLine]);
	Wait(appClerk.clerkCV[myLine], appClerk.clerkLock[myLine]);
	Signal(appClerk.clerkCV[myLine], appClerk.clerkLock[myLine]);
	Release(appClerk.clerkLock[myLine]); 
	*/
	Signal(clerkCV, clerkLock);
	Wait(clerkCV, clerkLock);
	Signal(clerkCV, clerkLock);
	Release(clerkLock);
}

int doPicClerkStuff(int socialSecurityNum, int *cash) {

	int myLine;
	int choice;
	int probablity;
	int clerkLock;
	int clerkCV;

	myLine = getInLine(&picClerk,socialSecurityNum,cash);
	choice = 0;
	clerkLock = GetMV(picClerk.clerkLock, myLine);
	clerkCV = GetMV(picClerk.clerkCV, myLine);

	/*now we must obtain the lock from the PicClerk which went to wait state once he was avaiable and 
	waiting for a customer to signal him*/
	/*Acquire(picClerk.clerkLock[myLine]);*/
	Acquire(clerkLock);
	/*picClerk.currentCustomer[myLine] = socialSecurityNum;*/ /*which customer is currently being served*/
	SetMV(picClerk.currentCustomer, myLine, socialSecurityNum);

	Uprintf("Customer #%d has given SSN %d to PictureClerk #%d.\n", 51, socialSecurityNum, socialSecurityNum, myLine, 0);
	/*Signal(picClerk.clerkCV[myLine], picClerk.clerkLock[myLine]);*/
	Signal(clerkCV, clerkLock);
	/*wait for the clerk to confirm then we decide if we like the photo or not*/
	/*Wait(picClerk.clerkCV[myLine], picClerk.clerkLock[myLine]);*/
	Wait(clerkCV, clerkLock);
	probablity = Rand(10, 0); /*generate a random # from 0 - 9, if less than or equal to 4, we retake photo*/
	probablity = 5;

	if(probablity <= 4) { /*if we disliked the photo*/
			Uprintf("Customer #%d does not like their picture from PictureClerk #%d.\n", 64, socialSecurityNum, myLine, 0, 0);
			/*customersWithCompletedPics[socialSecurityNum] = 0;*/
			SetMV(customersWithCompletedPics,socialSecurityNum, 0);
			choice = 1;
	} else {
			Uprintf("Customer #%d likes their picture from PictureClerk #%d.\n", 61, socialSecurityNum, myLine, 0, 0);
			/*customersWithCompletedPics[socialSecurityNum] = 1;*/
			SetMV(customersWithCompletedPics,socialSecurityNum, 1);
			choice = 0;		
	}

	/*
	Signal(picClerk.clerkCV[myLine], picClerk.clerkLock[myLine]);	
	Release(picClerk.clerkLock[myLine]);
	*/
	Signal(clerkCV, clerkLock);
	Release(clerkLock);
	return choice;
}

void doPassportClerkStuff(int socialSecurityNum,int*cash){

	int mySSN;
	int myLine;
	int workLock;
	int workCV;
	int myPassportChecked;

	mySSN = socialSecurityNum;

	/*First get in line with a generic method*/
	myLine = getInLine(&passPClerk,socialSecurityNum,cash);
	
	/*
	workLock = passPClerk.clerkLock[myLine];
	workCV = passPClerk.clerkCV[myLine];
	*/

	workLock = GetMV(passPClerk.clerkLock, myLine);
	workCV = GetMV(passPClerk.clerkCV, myLine);
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
	/*myPassportChecked = passportClerkChecked[mySSN];*/
	myPassportChecked = GetMV(passportClerkChecked, mySSN);
	
	if(myPassportChecked == 0) {
		Uprintf("Customer #%d has gone to PassportClerk #%d too soon. They are going to the back of the line.\n", 93, socialSecurityNum, myLine,0, 0);
		punish(punishTime);
	}
	return;
}

void doCashierStuff(int mySSN, int* cash){

	int socialSecurityNum;
	int myLine;
	int workLock;
	int workCV;
	int readyToPay;

	socialSecurityNum = mySSN;
	
	/*First get in line with a generic method*/
	myLine = getInLine(&cashier,socialSecurityNum,cash);
	/*
	workLock = cashier.clerkLock[myLine];
	workCV = cashier.clerkCV[myLine];
	*/
	workLock = GetMV(cashier.clerkLock, myLine);
	workCV = GetMV(cashier.clerkCV, myLine);
	Acquire(workLock);

	/*Tell Clerk CV, then wait*/
	Uprintf("Customer #%d has given SSN %d to Cashier #%d.\n", 47, socialSecurityNum, socialSecurityNum, myLine, 0);
	tellCashierSSN(mySSN,myLine);
	Signal(workCV, workLock);
	Wait(workCV, workLock);

	/*Decide weather to self-punish*/
	/*readyToPay = cashierChecked[mySSN];*/
	readyToPay = GetMV(cashierChecked, mySSN);	
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
	/*Broadcast to all clerk lines so that custs wake up*/
	Acquire(appClerk.lineLock);
	/* Broadcast(appClerk.lineCV[0], appClerk.lineLock); */
	Broadcast(GetMV(appClerk.lineCV, 0), appClerk.lineLock);
	Release(appClerk.lineLock);

	Acquire(picClerk.lineLock);
	/* Broadcast(picClerk.lineCV[0], picClerk.lineLock); */
	Broadcast(GetMV(picClerk.lineCV, 0), picClerk.lineLock);	
	Release(picClerk.lineLock);

	Acquire(passPClerk.lineLock);
	/* Broadcast(passPClerk.lineCV[0], passPClerk.lineLock); */
	Broadcast(GetMV(passPClerk.lineCV, 0), passPClerk.lineLock);		
	Release(passPClerk.lineLock);

	Acquire(cashier.lineLock);
	/* Broadcast(cashier.lineCV[0], cashier.lineLock); */
	Broadcast(GetMV(cashier.lineCV, 0), cashier.lineLock);			
	Release(cashier.lineLock);			
}

int main() {

	/*Customer variables*/
	int cash;
	int appClerkSeen;
	int picClerkSeen;
	int myLine;
	int socialSecurityNum;
	int picOrAppClerk;
	int canStartWorking;
	int senatorWaitTime;
	int notCompleted;
	int mistake;
	int totalSales;
	int social;
	int ifSenator;
	int senWorking; 
	int ifCompletedApp;
	int ifCompletedPic;
	int ifCompletedPass;
	int ifCompletedCash;
	int numOfCustsLeft;
	int ifBribesEnabled;

	appClerkSeen = 0;
	picClerkSeen = 0;
	cash = 1100;
	canStartWorking = 1;
	
	createServerMVs(1,1);
	initialize(&picClerk, 1, 1);
	initialize(&appClerk, 0, 1);
	initialize(&passPClerk, 2, 1);
	initialize(&cashier, 3, 1);

	numOfCustsLeft = GetMV(numCustomersLeft, 0);
	/*Uprintf("%d customers left\n", 45, numOfCustsLeft, 0, 0, 0);*/


	/*Write("Here",4,ConsoleOutput);*/
	Acquire(newCustomerIdLock);
	/*
	social = newCustomerId;
	newCustomerId++;
	*/
	social = GetMV(newCustomerId, 0);
	SetMV(newCustomerId, 0, social + 1);
	Release(newCustomerIdLock);

	ifSenator = GetMV(isSenator, social);
	/*Uprintf("ifSenator = %d \n", 16, ifSenator, 0, 0, 0);*/

	if (ifSenator == 1) {
		canStartWorking = 0;
	}

	while(canStartWorking  == 0) {
		if (ifSenator == 1) {
			senatorWaitTime = 3;
			/*Uprintf("ifSenator = %d \n", 16, ifSenator, 0, 0, 0);*/

			Acquire(senatorLock);
			senWorking = GetMV(senatorWorking, 0);
			/*Uprintf("senWorking = %d \n", 17, senWorking, 0, 0, 0);*/

			if (senWorking == NULL) {
				/*senatorWorking = social;*/
				SetMV(senatorWorking, 0, social);
				senWorking = social;
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
	
	notCompleted = 1;

	while(notCompleted == 1) {

		/*Stop if there's a senator trying to work*/
		Acquire(senatorLock);
		senWorking = GetMV(senatorWorking, 0);
		if(senWorking!=NULL && senWorking!=social){
			Wait(senatorCV, senatorLock);
		}

		Release(senatorLock);

		/*ERROR CASE: pick the wrong behavior, go ahead and get punished*/
		mistake = Rand(100,0);

		if(mistake==0){
			/*Which behavior will the customer pick*/
			int choice;
			choice = Rand(2,0);

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
		ifCompletedApp = GetMV(customersWithCompletedApps, socialSecurityNum);
		ifCompletedPic = GetMV(customersWithCompletedPics, socialSecurityNum);
		if(ifCompletedApp == 0 || ifCompletedPic == 0) {
			/*enter if we choose to go to the appClerk*/
			if (picOrAppClerk == 0) {
				doAppClerkStuff(socialSecurityNum,&cash);
				picOrAppClerk = 1;
				ifCompletedApp = GetMV(customersWithCompletedApps, socialSecurityNum);
				ifCompletedPic = GetMV(customersWithCompletedPics, socialSecurityNum);
				ifCompletedPass = GetMV(passportClerkChecked, socialSecurityNum);				
				continue;
			}

			/*enter if we choose to go to the picClerk*/
			if (picOrAppClerk == 1) {
				picOrAppClerk = doPicClerkStuff(socialSecurityNum,&cash);
				ifCompletedApp = GetMV(customersWithCompletedApps, socialSecurityNum);
				ifCompletedPic = GetMV(customersWithCompletedPics, socialSecurityNum);
				ifCompletedPass = GetMV(passportClerkChecked, socialSecurityNum);				
				continue;
			}
		}
		/*Do the passportClerk stuff*/
		else if(ifCompletedPass == 0){
			doPassportClerkStuff(socialSecurityNum,&cash);
			ifCompletedPass = GetMV(passportClerkChecked, socialSecurityNum);
			ifCompletedCash = GetMV(cashierChecked, socialSecurityNum);
			continue;
		}
		/*Do the cashier stuff*/
		else if(ifCompletedCash == 0){
			doCashierStuff(socialSecurityNum,&cash);
			ifCompletedCash = GetMV(cashierChecked, socialSecurityNum);
			continue;
		} else{
			notCompleted = 0;
		}
	}

	/*Clean up, let people wake up*/
	if(ifSenator == 1){
		Uprintf("Customer (Senator)#%d is leaving the Passport Office.\n", 54, socialSecurityNum, 0, 0, 0);
		/*senatorWorking = NULL;*/
		SetMV(senatorWorking, 0, NULL);
		senWorking = NULL;
		Acquire(senatorLock);
		Broadcast(senatorCV, senatorLock);
		Release(senatorLock);
	} else {
		Uprintf("Customer #%d is leaving the Passport Office.\n", 45, socialSecurityNum, 0, 0, 0);
	}
	/*numCustomersLeft-=1;*/
	numOfCustsLeft = GetMV(numCustomersLeft, 0);
	numOfCustsLeft-=1;
	SetMV(numCustomersLeft, 0, numOfCustsLeft);

	Uprintf("%d customers left\n", 45, numOfCustsLeft, 0, 0, 0);
	ifBribesEnabled = GetMV(bribesEnabled, 0);
	if(numOfCustsLeft==0 && ifBribesEnabled == 0){
		/*totalSales = appClerk.cashReceived + picClerk.cashReceived +
						 passPClerk.cashReceived + cashier.cashReceived;*/
		totalSales = GetMV(appClerk.cashReceived, 0) + GetMV(picClerk.cashReceived, 0) + GetMV(passPClerk.cashReceived, 0) + GetMV(cashier.cashReceived, 0);
		Uprintf("TOTAL SALES: %d\n", 16, totalSales, 0,0,0);
	}
	Exit(0);
	return 1;
}
