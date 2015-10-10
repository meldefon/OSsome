#include <time.h>
#include <iostream>
#include <stdlib.h>
#include "synch.h"
#include "globalVars.h"
using namespace std;

int punishTime = 100;

Monitor appClerk, picClerk, passPClerk, cashier;

//global shared data between the clerks that are used for filing purposes
bool *customersWithCompletedApps;
//int *customersWithCompletedApps;
bool *customersWithCompletedPics;
//int *customersWithCompletedPics;
bool *passportClerkChecked;
//int *passportClerkChecked;
bool *cashierChecked;
//int *cashierChecked;
bool *gottenPassport;
//int *gottenPassport;
int *cashReceived;
bool bribesEnabled;
//int bribesEnabled;

int* appClerkCurrentCustomer;
int* pictureClerkCurrentCustomer;
int* passportClerkCurrentCustomer;
int* cashierCurrentCustomer;

Lock* senatorLock;
//int senatorLock;
Condition* senatorCV;
//int senatorCV;
bool* isSenator;
//int* isSenator;
int senatorWorking;
bool clerksCanWork;
//int clerksCanWork;

int numCustomersLeft;

void punish(int time){
	for (int i = 0; i < time; i++) {
		currentThread->Yield();
	}
}

void tellPassportClerkSSN(int SSN,int myLine){
	//cout<<"Telling the passportClerk my SSN is "<<SSN<"\n";
	passportClerkCurrentCustomer[myLine] = SSN;
	//*(passportClerkCurrentCustomer + (myLine * sizeof(int))) = SSN;
}

void tellCashierSSN(int SSN, int myLine){
	cashierCurrentCustomer[myLine] = SSN;
	//*(cashierCurrentCustomer + (myLine * sizeof(int))) = SSN;
}

void payCashier(int SSN, int *cash){
	cashReceived[SSN]+=100;
	//*(cashierCurrentCustomer + (SSN * sizeof(int)))+= 100;
	cash-=100;
}


int getInLine(Monitor *clerk, int socialSecurityNum, int* cash) {

	if(senatorWorking!=NULL && senatorWorking==socialSecurityNum){
		//cout<<"Senator attemting to enter line\n";
		clerk->lineLock->Acquire();
		//Acquire(clerk->lineLock);
		clerk->senLineCount[0]++;
		//*(clerk->senLineCount)++;
		clerk->senLineCV[0].Wait(clerk->lineLock);
		//Wait(*(clerk->senLineCV), clerk->lineLock);
		clerk->senLineCount[0]--;
		//*(clerk->senLineCount)--;
		clerk->lineLock->Release();
		//Release(clerk->lineLock);
		return 0;
	}

		int* lineCount;
		Condition* lineCV;
		//int lineCV;
		bool didBribe = false;
		//int didBribe = 0;

		int wantToBribe = rand() % 10; // random choice about whether to bribe
		if(strcmp(clerk->clerkType,"Cashier")){
			wantToBribe = 1;
		}
		//bool wantToBribe = socialSecurityNum==4;
		if(wantToBribe==0 && *cash>100 && bribesEnabled /* bribesEnabled == 1*/){
			*cash-=500;
			lineCount = clerk->bribeLineCount;
			lineCV = clerk->bribeLineCV;
			didBribe = true;
			//didBribe = 1;
		}
		else {
			lineCount = clerk->lineCount;
			lineCV = clerk->lineCV;
		}


		//possible clerk states are:
		//0 = busy
		//1 = break
		//2 = avaiable
		clerk->lineLock->Acquire();  //grab the lock since we are dealing with shared data for the lines and clerks
		//Acquire(clerk->lineLock);

		//pick the shortest clerk line
		//cout<<"Customer #" << socialSecurityNum << " has acquired the "<<clerk->lineLock->getName()<<". Bribe:"<<wantToBribe<<"\n";

		int myLine = -1;
		//int sizeOfInt = sizeof(int);
		while(myLine == -1) { //if we haven't found a line, we need to loop back again

			int lineSize = 777;

			for(int i = 0; i < clerk->numOfClerks; i++) {
				
				if(clerk->clerkState[i] == 2 /* *(clerk->clerkState + (i * sizeOfInt)) == 2*/) { //or the clerk is available
					myLine = i;
					lineSize = lineCount[i];
					//lineSize = *(lineCount + (i * sizeOfInt));
					break; //leave since we found the right clerk
				}
				//Right now, customer picks shortest line regardless of break status
				else if(lineCount[i] < lineSize/* *(lineCount + (i * sizeOfInt)) < lineSize*/) { //if the line we are in is the
					// shortest and the clerk is not on break, consider that line
					myLine = i;
					lineSize = lineCount[i];
					//lineSize = *(lineCount + (i * sizeOfInt));
				}
			}
			
			//if all the clerks on are on break
			if(myLine == -1) {
				cout<<"Customer #" << socialSecurityNum << " has entered "<<clerk->clerkType<<" Limbo Line.\n";
				clerk->numCustomersInLimbo++;
				clerk->limboLineCV->Wait(clerk->lineLock); //wait to be signaled by manager to wake up from limbo line
				//Wait(clerk->limboLineCV, clerk->lineLock);
			}
		}
		
		if(clerk->clerkState[myLine] == 0 || clerk->clerkState[myLine]==1 /*clerk->*(clerkState + (myLine * sizeOfInt)) == 0 || clerk->*(clerkState + (myLine * sizeOfInt)) == 1*/) { //if the clerk is busy with another customer, we must wait, else just
			// bypass this and go straight to transaction
			if(!didBribe /*didBribe == 0 */)
				cout<<"Customer #" << socialSecurityNum << " has gotten in regular line for "<<clerk->clerkType<<" #" << myLine << ".\n";
			else
				cout<<"Customer #" << socialSecurityNum << " has gotten in bribe line for "<<clerk->clerkType<<" #" << myLine << ".\n";
			
			lineCount[myLine]++; //get in line
			//*(lineCount + (myLine * sizeOfInt))++;
			lineCV[myLine].Wait(clerk->lineLock); //wait until we are signaled by AppClerk
			//Wait(*(lineCV + (myLine * sizeOfInt)), clerk->lineLock);
			lineCount[myLine]--; //get out of line and move to the counter
			//*(lineCount + (myLine * sizeOfInt))--;

		}

			
		clerk->clerkState[myLine] = 0;   //set the clerk status to busy
		//*(clerk->clerkState + (myLine * sizeOfInt)) = 0;
		clerk->lineLock->Release(); //release the lock since we now can begin to conduct the transaction 
									//which is only relavent to the two threads of customer and clerk
		//Release(clerk->lineLock);
		//cout<<"Customer #" << socialSecurityNum << " has released the "<<clerk->clerkType<<" Clerk Line Lock!\n";

	return myLine;
}

void doAppClerkStuff(int socialSecurityNum, int* cash) {

	int myLine = getInLine(&appClerk,socialSecurityNum, cash);
	//int sizeOfInt = sizeof(int);

	//now we must obtain the lock from the AppClerk which went to wait state once he was avaiable and 
	//waiting for a customer to signal him
	appClerk.clerkLock[myLine].Acquire();
	//Acquire(*(appClerk.clerkLock + (myLine * sizeOfInt)));
	//cout<<"Customer #" << socialSecurityNum << " has acquired the Application Clerk #" << myLine << " Condition Variable Lock!\n";
	
	appClerk.currentCustomer[myLine] = socialSecurityNum; //which customer is currently being served
	//*(appClerk.currentCustomer + (myLine * sizeOfInt)) = socialSecurityNum;
	cout<<"Customer #" <<socialSecurityNum << " has given SSN "<< socialSecurityNum << " to ApplicationClerk #" << myLine << ".\n";	

	appClerk.clerkCV[myLine].Signal(&(appClerk.clerkLock[myLine])); //signal the AppClerk that we have given him the social security number
	//Signal(*(appClerk.clerkCV + (myLine * sizeOfInt)), *(appClerk.clerkLock + (myLine * sizeOfInt)));
	appClerk.clerkCV[myLine].Wait(&(appClerk.clerkLock[myLine])); 	//wait for the clerk to confirm
	//Wait(*(appClerk.clerkCV + (myLine * sizeOfInt)), *(appClerk.clerkLock + (myLine * sizeOfInt)));
	appClerk.clerkCV[myLine].Signal(&(appClerk.clerkLock[myLine])); //signal the clerk
	//Signal(*(appClerk.clerkCV + (myLine * sizeOfInt)), *(appClerk.clerkLock + (myLine * sizeOfInt)));

	appClerk.clerkLock[myLine].Release(); //let go of the lock
	//Release(*(appClerk.clerkLock + (myLine * sizeOfInt)));
	//cout<<"Customer #" << socialSecurityNum << " has released the Application Clerk #" << myLine << " Condition Variable Lock!\n";
}

int doPicClerkStuff(int socialSecurityNum, int* cash) {

	int myLine = getInLine(&picClerk,socialSecurityNum,cash);
	int choice = 0;
	//int sizeOfInt = sizeof(int);

	//now we must obtain the lock from the PicClerk which went to wait state once he was avaiable and 
	//waiting for a customer to signal him
	picClerk.clerkLock[myLine].Acquire();
	//Acquire(*(picClerk.clerkLock + (myLine * sizeOfInt)));
	picClerk.currentCustomer[myLine] = socialSecurityNum; //which customer is currently being served
	//*(picClerk.currentCustomer + (myLine * sizeOfInt))) = socialSecurityNum;
	cout<<"Customer #" <<socialSecurityNum << " has given SSN "<< socialSecurityNum << " to PictureClerk #" << myLine << ".\n";	

	//cout<<"Customer #" << socialSecurityNum << " has acquired the Picture Clerk #" << myLine << " Condition Variable Lock!\n";

	//signal the PicClerk that we have given him the social security number
	picClerk.clerkCV[myLine].Signal(&(picClerk.clerkLock[myLine]));
	//Signal(*(picClerk.clerkCV + (myLine * sizeOfInt)), *(picClerk.clerkLock + (myLine * sizeOfInt)));
	//wait for the clerk to confirm then we decide if we like the photo or not
	picClerk.clerkCV[myLine].Wait(&(picClerk.clerkLock[myLine]));
	//Wait(*(picClerk.clerkCV + (myLine * sizeOfInt)), *(picClerk.clerkLock + (myLine * sizeOfInt)));

	int probablity = rand() % 10; //generate a random # from 0 - 9, if less than or equal to 4, we retake photo

	if(probablity <= 4) { //if we disliked the photo
			cout<<"Customer #" << socialSecurityNum << " does not like their picture from Picture Clerk #" << myLine << ".\n";
			customersWithCompletedPics[socialSecurityNum] = false;
			//*(customersWithCompletedPics + (socialSecurityNum * sizeOfInt)) = 0;
			choice = 1;
	} else {
			cout<<"Customer #" << socialSecurityNum << " likes their picture from Picture Clerk #" << myLine << ".\n";
			customersWithCompletedPics[socialSecurityNum] = true;
			//*(customersWithCompletedPics + (socialSecurityNum * sizeOfInt)) = 1;
			choice = 0;		
	}

	picClerk.clerkCV[myLine].Signal(&(picClerk.clerkLock[myLine])); //signal the clerk
	//Signal(*(picClerk.clerkCV + (myLine * sizeOfInt)), *(picClerk.clerkLock + (myLine * sizeOfInt)));	
	picClerk.clerkLock[myLine].Release(); //let go of the lock
	//Release(*(picClerk.clerkLock + (myLine * sizeOfInt)));
	
	return choice;
	//cout<<"Customer #" << socialSecurityNum << " has released the Picture Clerk #" << myLine << " Condition Variable Lock!\n";
}

void doPassportClerkStuff(int socialSecurityNum,int*cash){

	int mySSN = socialSecurityNum;
	//int sizeOfInt = sizeof(int);

	//First get in line with a generic method
	//cout<<"Customer #"<<socialSecurityNum<<" getting in passport Line\n";
	int myLine = getInLine(&passPClerk,socialSecurityNum,cash);

	//Enter interaction monitor with passport clerk
	Lock* workLock = &passPClerk.clerkLock[myLine];
	//int workLock = *(passPClerk.clerkLock + (myLine * sizeOfInt));
	Condition* workCV = &passPClerk.clerkCV[myLine];
	//int workCV = *(passPClerk.clerkCV + (myLine * sizeOfInt));
	workLock->Acquire();
	//Acquire(workLock);

	//Tell Clerk CV, then wait
	cout<<"Customer #"<<socialSecurityNum<<" has given SSN "<<socialSecurityNum<<" to PassportClerk #"<<myLine<<"\n";
	tellPassportClerkSSN(mySSN,myLine);
	workCV->Signal(workLock);
	//Signal(workCV, workLock);
	workCV->Wait(workLock);
	//Wait(workCV, workLock);

	//Now leave
	//cout<<"Customer #"<<socialSecurityNum<<" leaving passportClerk #"<<myLine<<"\n";
	workCV->Signal(workLock);
	//Signal(workCV, workLock);
	workLock->Release();
	//Release(workLock);

	//Decide weather to self-punish
	bool myPassportChecked = passportClerkChecked[mySSN];
	//int myPassportChecked = *(passportClerkChecked + (mySSN * sizeOfInt));
	if(!myPassportChecked /*myPassportChecked == 0 */) {
		cout<<"Customer #"<<socialSecurityNum<<" has gone to PassportClerk #"<<myLine<<" too soon. "<<
				"They are going to the back of the line.\n";
		punish(punishTime);
	}
	return;

}

void doCashierStuff(int mySSN, int* cash){

	int socialSecurityNum = mySSN;
	//int sizeOfInt = sizeof(int);

	//First get in line with a generic method
	//cout<<"Customer #"<<socialSecurityNum<<" getting in cashier line\n";
	int myLine = getInLine(&cashier,socialSecurityNum,cash);

	//Enter interaction monitor with passport clerk
	Lock* workLock = &cashier.clerkLock[myLine];
	//int workLock = *(cashier.clerkLock + (myLine * sizeOfInt));
	Condition* workCV = &cashier.clerkCV[myLine];
	//int workCV = *(cashier.clerkCV + (myLine * sizeOfInt));
	workLock->Acquire();
	//Acquire(workLock);

	//Tell Clerk CV, then wait
	cout<<"Customer #"<<socialSecurityNum<<" has given SSN "<<socialSecurityNum<<" to Cashier #"<<myLine<<"\n";
	tellCashierSSN(mySSN,myLine);
	workCV->Signal(workLock);
	//Signal(workCV, workLock);
	workCV->Wait(workLock);
	//Wait(workCV, workLock);

	//Decide weather to self-punish
	bool readyToPay = cashierChecked[mySSN];
	//int readyToPay = *(cashierChecked + (mySSN * sizeOfInt));
	if(!readyToPay/*readyToPay == 0*/) {
		//Release, punish, and leave
		cout<<"Customer #"<<socialSecurityNum<<" has gone to Cashier #"<<myLine<<" too soon. "<<
				"They are going to the back of the line.\n";
		workCV->Signal(workLock);
		//Signal(workCV, workLock);
		workLock->Release();
		//Release(workLock);
		punish(punishTime);
		return;
	}

	//Now you can pay
	cout<<"Customer #"<<socialSecurityNum<<" has given Cashier #"<<myLine<<" $100\n";
	payCashier(mySSN,cash);
	workCV->Signal(workLock);
	//Signal(workCV, workLock);
	workCV->Wait(workLock);
	//Wait(workCV, workLock);

	//Now you've been woken up because you have the passport, so leave
	//cout<<"Customer #"<<socialSecurityNum<<" got passport and is leaving cashier #"<<myLine<<"\n";
	workCV->Signal(workLock);
	//Signal(workCV, workLock);
	workLock->Release();
	//Release(workLock);
	return;
}

void senatorClearLines(){
	clerksCanWork = false;
	Monitor* allMonitors = new Monitor[4];
	//Monitor* allMonitors = (Monitor*) malloc(4 * sizeof(Monitor));
	//int sizeOfMonitor = sizeof(Monitor);
	allMonitors[0] = appClerk;
	//*(allMonitors) = appClerk;
	allMonitors[1] = picClerk;
	//*(allMonitors + sizeOfMonitor) = picClerk;
	allMonitors[2] = passPClerk;
	//*(allMonitors + (2 * sizeOfMonitor)) = passPClerk;
	allMonitors[3] = cashier;
	//*(allMonitors + (3 * sizeOfMonitor)) = cashier;

	//Broadcast to all clerk lines so that custs wake up
	for(int i = 0;i<4;i++) {

		allMonitors[i].lineLock->Acquire();
		//Acquire(*(allMonitors + (i * sizeOfMonitor)).lineLock);
		allMonitors[i].lineCV->Broadcast(allMonitors[i].lineLock);
		//Broadcast(*(allMonitors + (i * sizeOfMonitor)).lineCV, *(allMonitors + (i * sizeOfMonitor)).lineLock);
		allMonitors[i].lineLock->Release();
		//Release(*(allMonitors + (i * sizeOfMonitor)).lineLock);
	}

}

void customer(int social) {

	//Customer variables
	int cash = 1100;
	//int sizeOfInt = sizeof(int);
	bool appClerkSeen = false;
	//int appClerkSeen = 0;
	bool picClerkSeen = false;
	//int picClerkSeen = 0;
	int myLine;
	int socialSecurityNum;
	int picOrAppClerk;

	bool canStartWorking = true;
	//int canStartWorking = 1;
	if (isSenator[social]/**(isSenator + (social * sizeOfInt)) == 1*/) {
		canStartWorking = false;
		//canStartWorking = 0;
	}

	while(!canStartWorking/*canStartWorking == 0*/) {
		if (isSenator[social]/**(isSenator + (social * sizeOfInt)) == 1*/) {
			int senatorWaitTime = 3;
			/*for (int i = 0; i < senatorWaitTime; i++) {
				currentThread->Yield();
			}*/
			senatorLock->Acquire();
			//Acquire(senatorLock);
			if (senatorWorking == NULL) {
				senatorWorking = social;
				canStartWorking = true;
			}
			else {
				senatorCV->Wait(senatorLock);
				//Wait(senatorCV, senatorLock);
			}
			senatorLock->Release();
			//Release(senatorLock);
		}
	}



	//senatorClearLines();

	
	socialSecurityNum = social;
	picOrAppClerk = 0; //rand() % 2; //0 for appClerk, 1 for picClerk, 2 for both completed, randomnly generated
	
	//set to true if we've seen these clerks so we can use it for PassportClerk
	bool notCompleted = true; //while we are not done with everything 
	//int notCompleted = 1;

	while(notCompleted /*notCompleted == 1*/) {

		//Stop if there's a senator trying to work
		senatorLock->Acquire();
		//Acquire(senatorLock);
		if(senatorWorking!=NULL && senatorWorking!=social){
			senatorCV->Wait(senatorLock);
			//Wait(senatorCV, senatorLock);
		}
		senatorLock->Release();
		//Release(senatorLock);

		//ERROR CASE: pick the wrong behavior, go ahead and get punished
		int mistake = rand()%100;
		if(mistake==0){
			//Which behavior will the customer pick
			int choice = rand()%2;
			if(choice==0){
				doPassportClerkStuff(socialSecurityNum,&cash);
				continue;
			}
			else{
				doCashierStuff(socialSecurityNum,&cash);
				continue;
			}
		}
		//Do the picture/application clerk stuff
		/* if(*(customersWithCompletedApps + (socialSecurityNum * sizeOfInt)) == 0 || *(customersWithCompletedPics + (socialSecurityNum * sizeOfInt)) == 0)*/
		if(!customersWithCompletedApps[socialSecurityNum] || !customersWithCompletedPics[socialSecurityNum]) {
			//enter if we choose to go to the appClerk
			if (picOrAppClerk == 0) {
				doAppClerkStuff(socialSecurityNum,&cash);
				picOrAppClerk = 1;
				continue;
			}

			//enter if we choose to go to the picClerk
			if (picOrAppClerk == 1) {
				picOrAppClerk = doPicClerkStuff(socialSecurityNum,&cash);
				continue;
			}
		}
		//Do the passportClerk stuff
		else if(/**(passportClerkChecked + (socialSecurityNum * sizeOfInt)) == 0*/!passportClerkChecked[socialSecurityNum]){
			doPassportClerkStuff(socialSecurityNum,&cash);
			continue;
		}
		//DO the cashier stuff
		else if(/**(cashierChecked + (socialSecurityNum * sizeOfInt)) == 0*/!cashierChecked[socialSecurityNum]){
			doCashierStuff(socialSecurityNum,&cash);
			continue;
		}
		else{
			notCompleted = false;
			//notCompleted = 0;
		}
	}

	//Clean up, let people wake up
	if(isSenator[social]/**(isSenator + (social * sizeOfInt)) == 1*/){
		cout<<"Customer (Senator) #"<<socialSecurityNum<<" is leaving the Passport Office.\n";
		//cout<<socialSecurityNum<<"\n";
		senatorWorking = NULL;
		senatorLock->Acquire();
		//Acquire(senatorLock);
		senatorCV->Broadcast(senatorLock);
		//Broadcast(senatorCV, senatorLock);
		senatorLock->Release();
		//Release(senatorLock);
		//senatorClearLines();
	}
	else {
		cout << "Customer #" << socialSecurityNum << " is leaving the Passport Office.\n";
		//cout<<socialSecurityNum<<"\n";
	}
	numCustomersLeft-=1;
	if(numCustomersLeft==0 && !bribesEnabled /*bribesEnabled == 1*/){
		int totalSales = appClerk.cashReceived + picClerk.cashReceived +
						 passPClerk.cashReceived + cashier.cashReceived;
		cout<<"TOTAL SALES: "<<totalSales<<"\n";
	}
}
