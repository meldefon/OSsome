#include "globalVars.h"
#include "synch.h"
#include <iostream>
#include <string.h>
using namespace std;


bool waitForLine(Monitor* clerk,int myLineID, bool firstTime){
	clerk->lineLock->Acquire(); //acquire line lock
	bool ifBribed = false;

	//if there is someone in the bribe line, signal them first
	if(clerk->bribeLineCount[myLineID] > 0) {
		clerk->bribeLineCV[myLineID].Signal(clerk->lineLock);
		ifBribed = true;
		clerk->clerkState[myLineID] = 0; //set state to busy
		cout<<clerk->clerkType<<" #" << myLineID << " has signalled a Customer to come to their counter.\n";
	} else if(clerk->lineCount[myLineID] > 0) { //signal someone in normal line
		clerk->lineCV[myLineID].Signal(clerk->lineLock);
		clerk->clerkState[myLineID] = 0; //set state to busy
		cout<<clerk->clerkType<<" #" << myLineID  << " has signalled a Customer to come to their counter.\n";
	} else { //no one is in either line, we must go to sleep
		
		if(!firstTime) {
			cout<<clerk->clerkType<<" #" << myLineID  << " is going on break.\n";
			clerk->clerkState[myLineID] = 1; //set state to break
			clerk->breakCV->Wait(clerk->lineLock); //wait for the manager to signal
			cout<<clerk->clerkType<<" #" << myLineID  << " is coming off break.\n";
		}

		clerk->clerkState[myLineID] = 2; //set state to available
		//clerk->clerkState[myLineID] = 0;
		//return;
	}

	//grab the clerkLock so we can properly signal the waiting customer
	//to avoid a race condition and guarentee the correct order of events
	clerk->clerkLock[myLineID].Acquire();
	//cout<<clerk->clerkType<<" Clerk #" << myLineID  << " obtained the "<<clerk->clerkType<<" Clerk Condition Variable Lock!\n";

	//now we can let go of line lock since we properly acquired the clerk lock
	clerk->lineLock->Release();
	//cout<<clerk->clerkType<<" Clerk #" << myLineID  << " released the "<<clerk->clerkType<<" Clerk Line Lock!\n";

	clerk->clerkCV[myLineID].Wait(&(clerk->clerkLock[myLineID])); //wait for customer to signal us
	return ifBribed;
	//cout<<clerk->clerkType<<" Clerk #" << myLineID  << " received customer's social security number!\n"; //ask now for cash
}

void pictureClerk(int id) {
	
	int myLineID = id; //the index we pass in will be used as id's for the clerks
	bool firstTime = true;
	bool ifBribed;

	while(true) {	
		//cout<<"Picture Clerk #" << id << " about to wait for customer\n";
		ifBribed = waitForLine(&picClerk, id, firstTime); 

		cout<<"PictureClerk #" << id << " has received SSN " << picClerk.currentCustomer[id] << " from Customer #" << picClerk.currentCustomer[id] << ".\n";
		cout<<"PictureClerk #" << id << " has taken a picture of Customer #" << picClerk.currentCustomer[id] << ".\n";

		picClerk.clerkCV[myLineID].Signal(&(picClerk.clerkLock[myLineID])); //tell the customer the picture is completed
		picClerk.clerkCV[myLineID].Wait(&(picClerk.clerkLock[myLineID])); //wait for customer to decide if they like the photo
		
		if(customersWithCompletedPics[picClerk.currentCustomer[id]] == true) {
			cout<<"PictureClerk #" << id << " has been told that Customer #" << picClerk.currentCustomer[id] << " does like their picture.\n";
		} else {
			cout<<"PictureClerk #" << id << " has been told that Customer #" << picClerk.currentCustomer[id] << " does not like their picture.\n";
		}

		if(ifBribed) {
			cout<<"PictureClerk #" << id << " has received $500 from Customer #" << picClerk.currentCustomer[id] << ".\n";
			picClerk.cashReceived+=500;
		}

		firstTime = false;
		picClerk.clerkLock[myLineID].Release(); //release the clerk lock since we are done
		//cout<<"Picture Clerk #" << id << " released the Picture Clerk Condition Variable Lock!\n";
	}
}

void applicationClerk(int id) {
	
	int myLineID = id; //the index we pass in will be used as id's for the clerks
	bool firstTime = true;
	bool ifBribed;

	while(true) {	
		//cout<<"Application Clerk #" << id << " about to wait for customer\n";
		waitForLine(&appClerk, id, firstTime); 

		cout<<"ApplicationClerk #" << id << " has received SSN " << appClerk.currentCustomer[id] << " from Customer #" << appClerk.currentCustomer[id] << ".\n";

		appClerk.clerkCV[myLineID].Signal(&(appClerk.clerkLock[myLineID])); //tell the customer their application is done
		appClerk.clerkCV[myLineID].Wait(&(appClerk.clerkLock[myLineID])); //wait for customer to leave us
		
		//input the socialSecurityNum into the completed applications
		customersWithCompletedApps[appClerk.currentCustomer[id]] = true;

		cout<<"ApplicationClerk #" << id << " has recorded a completed application for Customer #" << appClerk.currentCustomer[id] << ".\n";
		
		if(ifBribed) {
			cout<<"ApplicationClerk #" << id << " has received $500 from Customer #" << appClerk.currentCustomer[id] << ".\n";
			appClerk.cashReceived+=500;
		}

		firstTime = false;
		appClerk.clerkLock[myLineID].Release(); //release the clerk lock since we are done
		//cout<<"Application Clerk #" << id << " released the Application Clerk Condition Variable Lock!\n";
	}
}

void passportClerk(int id) {

	int myLineID = id; 	//set ID
	bool firstTime = true;
	bool ifBribed = false;

	while(true) {
		//Wait fot the next cust to signal
		cout << "PassportClerk #" << id << " about to wait for customer\n";
		ifBribed = waitForLine(&passPClerk, id, firstTime);

		//Set up some convenient variables
		Lock *workLock = &passPClerk.clerkLock[myLineID];
		Condition *workCV = &passPClerk.clerkCV[myLineID];

		//Now the clerk has been woken up and has been told the customer ID
		//Check
		int customerSSN = passportClerkCurrentCustomer[myLineID];
		cout << "PassportClerk #" << id << " has recieved SSN " << customerSSN
			<<" from Customer #"<<customerSSN<<"\n";
		passportClerkChecked[customerSSN] =
				customersWithCompletedApps[customerSSN] && customersWithCompletedPics[customerSSN];
		if(passportClerkChecked[customerSSN]){
			cout<<"PassportClerk #" << id << " has determined that Customer #"<<customerSSN<<
					" has both their application and picture completed\n";
		}
		else{
			cout<<"PassportClerk #" << id << " has determined that Customer #"<<customerSSN<<
			" does not have both their application and picture completed\n";
		}
		//And Signal
		workCV->Signal(workLock);
		workCV->Wait(workLock);

		if(ifBribed){
			cout<<"PassportClerk #" << id << " has received $500 from Customer #" << customerSSN << ".\n";
			passPClerk.cashReceived+=500;
		}

		//Now customer is gone
		//cout << "Passport Clerk #" << id << " has recorded Customer #" << customerSSN << " passport documentation\n";
		firstTime = false;
		workLock->Release();
	}
}

void cashierDo(int id) {

	int myLineID = id; 	//set ID
	bool firstTime = true;
	bool ifBribed;

	while (true) {
		//Wait fot the next customer to signal
		cout << "Cashier #" << id << " about to wait for customer\n";
		ifBribed = waitForLine(&cashier, id, firstTime);

		//Set up some convenient variables
		Lock *workLock = &cashier.clerkLock[myLineID];
		Condition *workCV = &cashier.clerkCV[myLineID];

		//Now the clerk has been woken up and has been told the customer ID
		//Check
		int customerSSN = cashierCurrentCustomer[myLineID];
		cout << "Cashier #" << id << " has recieved SSN " << customerSSN <<
				" from Customer #"<<customerSSN<<"\n";
		cashierChecked[customerSSN] = passportClerkChecked[customerSSN];
		//And Signal
		workCV->Signal(workLock);
		workCV->Wait(workLock);

		if (!cashierChecked[customerSSN]) {
			//Now customer is gone
			cout<<"Cashier #"<<id<<" has recieved $100 from Customer #"<<customerSSN<<
					" before certification. They are to go to the back of the line\n";
			workLock->Release();
			return;
		}

		//Now customer has paid, so give passport and mark
		cout<<"Cashier #"<<id<<" has verified that Customer #"<<customerSSN<<
				" has been certified by a PassportClerk\n";
		cout<<"Cashier #"<<id<<" has recieved $100 from Customer #"<<customerSSN<<
				" after certification\n";
		cout<<"Cashier #"<<id<<" has provided Customer #"<<customerSSN<<" their complete passport\n";
		gottenPassport[customerSSN] = true;
		workCV->Signal(workLock);
		workCV->Wait(workLock);

		//Now customer has left
		cout<<"Cashier #"<<id<<" has recorded that Customer #"<<customerSSN<<
				" has been given their completed passport\n";
		firstTime = false;
		workLock->Release();
	}

}

void checkForClerkOnBreak(Monitor *clerk) {

	clerk->lineLock->Acquire(); //acquire the line lock 
	bool clerksOnBreak = false;

	//check if there are any clerks on break
	for(int i = 0; i <clerk->numOfClerks; i++) {
		if(clerk->clerkState[i] == 1) {
			clerksOnBreak = true;
			break;
		}
	}

	if(clerksOnBreak) {
		//check if there is a particular line with more than 3 customers waiting
		for(int i = 0; i < clerk->numOfClerks; i++) {
			if(clerk->lineCount[i] > 3 || clerk->bribeLineCount[i] > 3) {
				
				if(clerk->numCustomersInLimbo != 0) {
					clerk->limboLineCV->Broadcast(clerk->lineLock); //wake up all sleeping customers
					clerk->numCustomersInLimbo = 0;
				}

				for(int j = 0; j < clerk->numOfClerks; j++) {
					
					if(clerk->clerkState[j] == 1) { //if a clerk is on break, wake them up
						clerk->breakCV->Signal(clerk->lineLock);

						if(strcmp(clerk->clerkType, "ApplicationClerk") == 0)
							cout<<"Manager has woken up an ApplicationClerk.\n";
						else
							cout<<"Manager has woken up a " << clerk->clerkType << ".\n";

					}
				}

				break;
			}
		}
	}

	clerk->lineLock->Release(); //release the line lock
}

void managerDo(int id) {

	int myID = id; 	//set ID

	while (true) {
		checkForClerkOnBreak(&appClerk);
		checkForClerkOnBreak(&picClerk);
		checkForClerkOnBreak(&passPClerk);
		checkForClerkOnBreak(&cashier);

		cout<<"Manager has counted a total of $" << appClerk.cashReceived << " for ApplicationClerks.\n";
		cout<<"Manager has counted a total of $" << picClerk.cashReceived << " for PictureClerks.\n";
		cout<<"Manager has counted a total of $" << passPClerk.cashReceived << " for PassportClerks.\n";
		cout<<"Manager has counted a total of $" << cashier.cashReceived << " for Cashiers.\n";
		cout<<"Manager has counted a total of $" << appClerk.cashReceived + picClerk.cashReceived + passPClerk.cashReceived + cashier.cashReceived << " for the passport office.\n";
		
		//go on "break" per say by only checking periodically
		for(int i = 0; i = 20; i++) {
			currentThread->Yield();
		}
	}	
}
