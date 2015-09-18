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
bool *customersWithCompletedPics;
bool *passportClerkChecked;
bool *cashierChecked;
bool *gottenPassport;
int *cashReceived;

int* appClerkCurrentCustomer;
int* pictureClerkCurrentCustomer;
int* passportClerkCurrentCustomer;
int* cashierCurrentCustomer;



void punish(int time){
	for (int i = 0; i < time; i++) {
		currentThread->Yield();
	}
}

void tellPassportClerkSSN(int SSN,int myLine){
	//cout<<"Telling the passportClerk my SSN is "<<SSN<"\n";
	passportClerkCurrentCustomer[myLine] = SSN;
}

void tellCashierSSN(int SSN, int myLine){
	cashierCurrentCustomer[myLine] = SSN;
}

void payCashier(int SSN, int *cash){
	cashReceived[SSN]+=100;
	cash-=100;
}


int getInLine(Monitor *clerk, int socialSecurityNum, int* cash) {

		int* lineCount;
		Condition* lineCV;

		//int wantToBribe = 1;// rand() % 2; // random choice about whether to bribe
		bool wantToBribe = socialSecurityNum==4;
		if(wantToBribe && *cash>100){
			*cash-=500;
			lineCount = clerk->bribeLineCount;
			lineCV = clerk->bribeLineCV;
		}
		else{
			lineCount = clerk->lineCount;
			lineCV = clerk->lineCV;
		}


		//possible clerk states are:
		//0 = busy
		//1 = break
		//2 = avaiable
		clerk->lineLock->Acquire();  //grab the lock since we are dealing with shared data for the lines and clerks
		
		//pick the shortest clerk line
		cout<<"Customer #" << socialSecurityNum << " has acquired the "<<clerk->lineLock->getName()<<". Bribe:"<<wantToBribe<<"\n";

		int myLine = -1;
		int lineSize = 777;

		for(int i = 0; i < clerk->numOfClerks; i++) {
			
			if(clerk->clerkState[i] == 2) { //or the clerk is available
				myLine = i;
				lineSize = lineCount[i];
				break; //leave since we found the right clerk
			} 
			else if(lineCount[i] < lineSize && clerk->clerkState[i] != 1) { //if the line we are in is the
				// shortest and the clerk is not on break, consider that line
				myLine = i;
				lineSize = lineCount[i];
			}
		}
		
		
		if(clerk->clerkState[myLine] == 0) { //if the clerk is busy with another customer, we must wait, else just
			// bypass this and go straight to transaction
			cout<<"Customer #" << socialSecurityNum << " is waiting for "<<clerk->clerkType<<" Clerk #" << myLine << "!\n";
			lineCount[myLine]++; //get in line
			lineCV[myLine].Wait(clerk->lineLock); //wait until we are signaled by AppClerk
			lineCount[myLine]--; //get out of line and move to the counter
		}
			
		clerk->clerkState[myLine] = 0;   //set the clerk status to busy
		cout<<"Customer #" << socialSecurityNum << " is being served by "<<clerk->clerkType<<" Clerk #" << myLine <<"!\n";
		
		clerk->lineLock->Release(); //release the lock since we now can begin to conduct the transaction 
									//which is only relavent to the two threads of customer and clerk
		cout<<"Customer #" << socialSecurityNum << " has released the "<<clerk->clerkType<<" Clerk Line Lock!\n";


	return myLine;
}

void doAppClerkStuff(int socialSecurityNum, int* cash) {

	int myLine = getInLine(&appClerk,socialSecurityNum, cash);

	//now we must obtain the lock from the AppClerk which went to wait state once he was avaiable and 
	//waiting for a customer to signal him
	appClerk.clerkLock[myLine].Acquire();
	cout<<"Customer #" << socialSecurityNum << " has acquired the Application Clerk #" << myLine << " Condition Variable Lock!\n";
	
	//input the socialSecurityNum into the completed applications
	customersWithCompletedApps[socialSecurityNum] = true;

	//signal the AppClerk that we have given him the social security number
	appClerk.clerkCV[myLine].Signal(&(appClerk.clerkLock[myLine]));

	//wait for the clerk to confirm then deduct cash after
	appClerk.clerkCV[myLine].Wait(&(appClerk.clerkLock[myLine]));
	
	*cash-=100; //deduct cash

	appClerk.clerkCV[myLine].Signal(&(appClerk.clerkLock[myLine])); //signal the clerk
	appClerk.clerkLock[myLine].Release(); //let go of the lock
	cout<<"Customer #" << socialSecurityNum << " has released the Application Clerk #" << myLine << " Condition Variable Lock!\n";
}

void doPicClerkStuff(int socialSecurityNum, int* cash) {

	int myLine = getInLine(&picClerk,socialSecurityNum);

	//now we must obtain the lock from the PicClerk which went to wait state once he was avaiable and 
	//waiting for a customer to signal him
	picClerk.clerkLock[myLine].Acquire();
	cout<<"Customer #" << socialSecurityNum << " has acquired the Picture Clerk #" << myLine << " Condition Variable Lock!\n";
	
	//input the socialSecurityNum into the completed applications
	customersWithCompletedPics[socialSecurityNum] = true;

	//signal the PicClerk that we have given him the social security number
	picClerk.clerkCV[myLine].Signal(&(picClerk.clerkLock[myLine]));

	//wait for the clerk to confirm then deduct cash after
	picClerk.clerkCV[myLine].Wait(&(picClerk.clerkLock[myLine]));
	
	*cash-=100; //deduct cash

	picClerk.clerkCV[myLine].Signal(&(picClerk.clerkLock[myLine])); //signal the clerk
	picClerk.clerkLock[myLine].Release(); //let go of the lock
	cout<<"Customer #" << socialSecurityNum << " has released the Picture Clerk #" << myLine << " Condition Variable Lock!\n";
}

void doPassportClerkStuff(int socialSecurityNum,int*cash){

	int mySSN = socialSecurityNum;

	//First get in line with a generic method
	cout<<"Customer #"<<socialSecurityNum<<" getting in passport Line\n";
	int myLine = getInLine(&passPClerk,socialSecurityNum,cash);

	//Enter interaction monitor with passport clerk
	Lock* workLock = &passPClerk.clerkLock[myLine];
	Condition* workCV = &passPClerk.clerkCV[myLine];
	workLock->Acquire();

	//Tell Clerk CV, then wait
	cout<<"Customer #"<<socialSecurityNum<<" telling passportClerk #"<<myLine<<" mySSN\n";
	tellPassportClerkSSN(mySSN,myLine);
	workCV->Signal(workLock);
	workCV->Wait(workLock);

	//Now leave
	cout<<"Customer #"<<socialSecurityNum<<" leaving passportClerk #"<<myLine<<"\n";
	workCV->Signal(workLock);
	workLock->Release();

	//Decide weather to self-punish
	bool myPassportChecked = passportClerkChecked[mySSN];
	if(!myPassportChecked) {
		punish(punishTime);
	}
	return;

}

void doCashierStuff(int mySSN, int* cash){

	int socialSecurityNum = mySSN;

	//First get in line with a generic method
	cout<<"Customer #"<<socialSecurityNum<<" getting in cashier line\n";
	int myLine = getInLine(&cashier,socialSecurityNum,cash);

	//Enter interaction monitor with passport clerk
	Lock* workLock = &cashier.clerkLock[myLine];
	Condition* workCV = &cashier.clerkCV[myLine];
	workLock->Acquire();

	//Tell Clerk CV, then wait
	cout<<"Customer #"<<socialSecurityNum<<" telling cashier #"<<myLine<<" mySSN\n";
	tellCashierSSN(mySSN,myLine);
	workCV->Signal(workLock);
	workCV->Wait(workLock);

	//Decide weather to self-punish
	bool readyToPay = cashierChecked[mySSN];
	if(!readyToPay) {
		//Release, punish, and leave
		cout<<"Customer #"<<socialSecurityNum<<" is self-punishing and leaving cashier #"<<myLine<<"\n";
		workCV->Signal(workLock);
		workLock->Release();
		punish(punishTime);
		return;
	}

	//Now you can pay
	cout<<"Customer #"<<socialSecurityNum<<" is paying cashier #"<<myLine<<"\n";
	payCashier(mySSN,cash);
	workCV->Signal(workLock);
	workCV->Wait(workLock);

	//Now you've been woken up because you have the passport, so leave
	cout<<"Customer #"<<socialSecurityNum<<" got passport and is leaving cashier #"<<myLine<<"\n";
	workCV->Signal(workLock);
	workLock->Release();

	return;

}





void customer(int social) {

	//Customer variables
	int cash = 1100;
	bool appClerkSeen = false;
	bool picClerkSeen = false;
	int myLine;
	int socialSecurityNum;
	int picOrAppClerk;


	
	socialSecurityNum = social;
	picOrAppClerk = 0; //rand() % 2; //0 for appClerk, 1 for picClerk, 2 for both completed, randomnly generated
	
	//set to true if we've seen these clerks so we can use it for PassportClerk
	bool notCompleted = true; //while we are not done with everything 

	while(notCompleted) {

		if(!customersWithCompletedApps[socialSecurityNum] || !customersWithCompletedPics[socialSecurityNum]) {
			//enter if we choose to go to the appClerk
			if (picOrAppClerk == 0) {
				cout << "Customer #" << socialSecurityNum << " is going to the Application Clerk Area!\n";
				doAppClerkStuff(socialSecurityNum,&cash);
				picOrAppClerk = 1;
				continue;
			}

			//enter if we choose to go to the picClerk
			if (picOrAppClerk == 1) {
				cout << "Customer #" << socialSecurityNum << " is going to the Picture Clerk Area!\n";
				doPicClerkStuff(socialSecurityNum,&cash);
				picOrAppClerk = 0;
				continue;
			}
		}

		else if(!passportClerkChecked[socialSecurityNum]){
			doPassportClerkStuff(socialSecurityNum,&cash);
			continue;
		}
		else if(!cashierChecked[socialSecurityNum]){
			doCashierStuff(socialSecurityNum,&cash);
			continue;
		}
		else{
			notCompleted = false;
		}
	}

}
