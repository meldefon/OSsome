#include <time.h>
#include <iostream>
#include "synch.h"
#include "globalVars.h"
using namespace std;

//globals for the customer functions
int cash = 1000;
bool appClerkSeen = false;
bool picClerkSeen = false; 
int myLine;	
int socialSecurityNum;
int picOrAppClerk;
int punishTime = 100;

Monitor appClerk, picClerk, passPClerk, cashier;

//global shared data between the clerks that are used for filing purposes
bool *customersWithCompletedApps;
bool *customersWithCompletedPics;
bool *passportClerkChecked;
bool *gottenPassport;
int *cashReceived;

int* appClerkCurrentCustomer;
int* pictureClerkCurrentCustomer;
int* passportClerkCurrentCustomer;
int* cashierCurrentCustomer;



void punish(int time){
	for (int i = 0; i < time; i++) {
		//CurrentThread->Yield();
	}
}

void tellPassportClerkSSN(int SSN){
	cout<<"Telling the passportClerk my SSN is "<<SSN<"\n";
	passportClerkCurrentCustomer[myLine] = SSN;
}


void getInLine(Monitor *clerk) {

		//possible clerk states are:
		//0 = busy
		//1 = break
		//2 = avaiable
		clerk->lineLock->Acquire();  //grab the lock since we are dealing with shared data for the lines and clerks
		
		//pick the shortest clerk line
		cout<<"Customer #" << socialSecurityNum << " has acquired the "<<clerk->lineLock->getName()<<"\n";

		myLine = -1;
		int lineSize = 777;

		for(int i = 0; i < clerk->numOfClerks; i++) {
			
			if(clerk->clerkState[i] == 2) { //or the clerk is available
				myLine = i;
				lineSize = clerk->lineCount[i];
				break; //leave since we found the right clerk
			} 
			else if(clerk->lineCount[i] < lineSize && clerk->clerkState[i] != 1) { //if the line we are in is the shortest and the clerk is not on break, consider that line
				myLine = i;
				lineSize = clerk->lineCount[i];
			}
		}
		
		
		if(clerk->clerkState[myLine] == 0) { //if the clerk is busy with another customer, we must wait, else just 									 //bypass this and go straight to transaction
			cout<<"Customer #" << socialSecurityNum << " is waiting for Application Clerk #" << myLine << "!\n";
			clerk->lineCount[myLine]++; //get in line
			clerk->lineCV[myLine].Wait(clerk->lineLock); //wait until we are signaled by AppClerk
			clerk->lineCount[myLine]--; //get out of line and move to the counter
		}
			
		clerk->clerkState[myLine] = 0;   //set the clerk status to busy
		cout<<"Customer #" << socialSecurityNum << " is being served by Application Clerk #" << myLine <<"!\n";
		
		clerk->lineLock->Release(); //release the lock since we now can begin to conduct the transaction 
									//which is only relavent to the two threads of customer and clerk
		cout<<"Customer #" << socialSecurityNum << " has released the Application Clerk Line Lock!\n";
}

void doAppClerkStuff() {

	getInLine(&appClerk);

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
	
	cash-=100; //deduct cash
	appClerkSeen = true; //we have seen the appClerk

	appClerk.clerkCV[myLine].Signal(&(appClerk.clerkLock[myLine])); //signal the clerk
	appClerk.clerkLock[myLine].Release(); //let go of the lock
	cout<<"Customer #" << socialSecurityNum << " has released the Application Clerk #" << myLine << " Condition Variable Lock!\n";

	if(picClerkSeen) {
		//go to passport or cashier clerk
	} else {
		picOrAppClerk = 1; //if we haven't see the picClerk, go see him
	}
}



void doPassportClerkStuff(){

	int mySSN = socialSecurityNum;

	//First get in line with a generic method
	//myLine = getInLine(passportClerkLineMonitor); //DISCUSS
	getInLine(&passPClerk);

	//Enter interaction monitor with passport clerk
	Lock* workLock = &passPClerk.clerkLock[myLine];
	Condition* workCV = &passPClerk.clerkCV[myLine];
	workLock->Acquire();

	//Tell Clerk CV, then wait
	tellPassportClerkSSN(mySSN);
	workCV->Signal(workLock);
	workCV->Wait(workLock);

	//Now leave
	workCV->Signal(workLock);
	workLock->Release();

	//Decide weather to self-punish
	bool myPassportChecked = passportClerkChecked[mySSN];
	if(!myPassportChecked) {
		punish(punishTime);
		return;
	}
	return;

}





void customer(int social) {
	
	socialSecurityNum = social;
	picOrAppClerk = 0; //rand() % 2; //0 for appClerk, 1 for picClerk, 2 for both completed, randomnly generated
	
	//set to true if we've seen these clerks so we can use it for PassportClerk
	bool notCompleted = true; //while we are not done with everything 

	while(notCompleted) {

		//enter if we choose to go to the appClerk
		if(picOrAppClerk == 0) {
			cout<<"Customer #" << socialSecurityNum << " is going to the Application Clerk Area!\n";
			doAppClerkStuff();
			notCompleted = false;
		}
		
		//enter if we choose to go to the picClerk
		if(picOrAppClerk == 1) {
			cout<<"Customer #" << socialSecurityNum << " is going to the Picture Clerk Area!\n";
			notCompleted = false;
		}
	}
}
