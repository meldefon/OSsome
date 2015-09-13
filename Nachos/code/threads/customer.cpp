#include <time.h>
#include <iostream>
#include "globalVars.h"
using namespace std;

//globals for the customer functions
int cash = 1000;
bool appClerkSeen = false;
bool picClerkSeen = false; 

void getInLine(Monitor *clerk) {

		//possible clerk states are:
		//0 = busy
		//1 = break
		//2 = avaiable

		clerk->lineLock->Acquire();  //grab the lock since we are dealing with shared data for the lines and clerks
		
		//pick the shortest clerk line
		int myLine = -1;
		int lineSize = 777;

		for(int i = 0; i < clerk->numOfClerks; i++) {
			
			//if the line we are in is the shortest and the clerk is not on break, consider that line
			if(clerk->lineCount[i] < lineSize && clerk->clerkState[i] != 1) {
				myLine = i;
				lineSize = clerk->lineCount[i];
			}
		}
		
		if(clerk->clerkState[myLine] == 0) { //if the clerk is busy with another customer, we must wait, else just 									 //bypass this and go straight to transaction
			clerk->lineCount[myLine]++; //get in line
			clerk->lineCV[myLine].Wait(clerk->lineLock); //wait until we are signaled by AppClerk
			clerk->lineCount[myLine]--; //get out of line and move to the counter
		}
			
		clerk->clerkState[myLine] = 0;   //set the clerk status to busy

		clerk->lineLock->Release(); //release the lock since we now can begin to conduct the transaction 
									//which is only relavent to the two threads of customer and clerk
}

void doAppClerkStuff() {

	getInLine(&appClerk);

	//now we must obtain the lock from the AppClerk which went to wait state once he was avaiable and 
	//waiting for a customer to signal him
	appClerk.clerkLock[myLine].Acquire();

	//input the socialSecurityNum into the completed applications
	customersWithCompletedApps[socialSecurityNum] = true;

	//signal the AppClerk that we have given him the social security number
	appClerk.clerkCV[myLine].Signal(&(appClerk->clerkLock[myLine]));

	//wait for the clerk to confirm then deduct cash after
	appClerk.clerkCV[myLine].Wait(&(appClerk->clerkLock[myLine]));
	
	cash-=100; //deduct cash
	appClerkSeen = true; //we have seen the appClerk

	appClerk.clerkCV[myLine].Signal(&(appClerk->clerkLock[myLine])); //signal the clerk
	appClerk.clerkLock[myLine].Release(); //let go of the lock

	if(picClerkSeen) {
		//go to passport or cashier clerk
	} else {
		picOrAppClerk = 1; //if we haven't see the picClerk, go see him
	}
}

void customer(int social) {
	
	int socialSecurityNum = social;
	int picOrAppClerk = rand() % 2; //0 for appClerk, 1 for picClerk, 2 for both completed, randomnly generated
	
	//set to true if we've seen these clerks so we can use it for PassportClerk
	bool notCompleted = true; //while we are not done with everything 

	while(notCompleted) {

		//enter if we choose to go to the appClerk
		if(picOrAppClerk == 0) {
			cout<<"Customer " << socialSecurityNum << " is going to the Application Clerk!\n";
			doAppClerkStuff();
		}
		
		//enter if we choose to go to the picClerk
		if(picOrAppClerk == 1) {
			cout<<"Customer " << socialSecurityNum << " is going to the Picture Clerk!\n";
		}
	}
}