#include "globalVars.h"
#include "synch.h"
#include <iostream>
using namespace std;


void waitForLine(Monitor* clerk,int myLineID){
	clerk->lineLock->Acquire(); //acquire line lock

	//if there is someone in the bribe line, signal them first
	if(clerk->bribeLineCount[myLineID] > 0) {
		//cout<<clerk->bribeLineCount[myLineID]<<"\n";
		clerk->bribeLineCV[myLineID].Signal(clerk->lineLock);
		clerk->clerkState[myLineID] = 0; //set state to busy
		//cout<<"Application Clerk #" << myLineID << " signaled someone in the bribe line!\n";
	} else if(clerk->lineCount[myLineID] > 0) { //signal someone in normal line
		clerk->lineCV[myLineID].Signal(clerk->lineLock);
		clerk->clerkState[myLineID] = 0; //set state to busy
		//cout<<"Application Clerk #" << myLineID  << " signaled someone in the line!\n";
	} else { //no one is in either line, we must go to sleep
		//cout<<"Application Clerk #" << myLineID  << " is available and waiting!\n";
		clerk->clerkState[myLineID] = 2; //set state to available
	}

	//grab the clerkLock so we can properly signal the waiting customer
	//to avoid a race condition and guarentee the correct order of events
	clerk->clerkLock[myLineID].Acquire();
	//cout<<"Application Clerk #" << myLineID  << " obtained the Application Clerk Condition Variable Lock!\n";

	//now we can let go of line lock since we properly acquired the clerk lock
	clerk->lineLock->Release();
	//cout<<"Application Clerk #" << myLineID  << " released the Application Clerk Line Lock!\n";

	clerk->clerkCV[myLineID].Wait(&(clerk->clerkLock[myLineID])); //wait for customer to signal us
	//cout<<"Application Clerk #" << myLineID  << " received customer's social security number!\n"; //ask now for cash


}

void applicationClerk(int id) {
	
	cout<<"Application Clerk #" << id << " is ready to go!\n";

	int myLineID = id; //the index we pass in will be used as id's for the clerks
					 //which also corresponds to the their line numbers
	while(true) {	
		appClerk.lineLock->Acquire(); //acquire the line lock
		cout<<"Application Clerk #" << id << " obtained the Application Clerk Line Lock!\n";
		//if there is someone in the bribe line, signal them first
		if(appClerk.bribeLineCount[myLineID] > 0) { 
			cout<<appClerk.bribeLineCount[myLineID]<<"\n";
			appClerk.bribeLineCV[myLineID].Signal(appClerk.lineLock);
			appClerk.clerkState[myLineID] = 0; //set state to busy
			cout<<"Application Clerk #" << id << " signaled someone in the bribe line!\n";
		} else if(appClerk.lineCount[myLineID] > 0) { //signal someone in normal line
			appClerk.lineCV[myLineID].Signal(appClerk.lineLock);
			appClerk.clerkState[myLineID] = 0; //set state to busy
			cout<<"Application Clerk #" << id << " signaled someone in the line!\n";
		} else { //no one is in either line, we must go to sleep
			cout<<"Application Clerk #" << id << " is available and waiting!\n";
			appClerk.clerkState[myLineID] = 2; //set state to available
		}

		//grab the clerkLock so we can properly signal the waiting customer
		//to avoid a race condition and guarentee the correct order of events
		appClerk.clerkLock[myLineID].Acquire();
		cout<<"Application Clerk #" << id << " obtained the Application Clerk Condition Variable Lock!\n";
		
		//now we can let go of line lock since we properly acquired the clerk lock
		appClerk.lineLock->Release();
		cout<<"Application Clerk #" << id << " released the Application Clerk Line Lock!\n";

		appClerk.clerkCV[myLineID].Wait(&(appClerk.clerkLock[myLineID])); //wait for customer to signal us
		cout<<"Application Clerk #" << id << " received customer's social security number!\n"; //ask now for cash
		
		appClerk.clerkCV[myLineID].Signal(&(appClerk.clerkLock[myLineID])); //tell the customer they need cash
		appClerk.clerkCV[myLineID].Wait(&(appClerk.clerkLock[myLineID])); //wait for customer to signal us
		cout<<"Application Clerk #" << id << " received customer's cash and is done!\n"; 
		
		appClerk.clerkLock[myLineID].Release(); //release the clerk lock since we are done
		cout<<"Application Clerk #" << id << " released the Application Clerk Condition Variable Lock!\n";
	}
}

void passportClerk(int id){

	//set ID
	int myLineID = id;

	//Wait fot the next cust to signal
	cout<<"Passport Clerk #"<<id<<" about to wait for customer\n";
	waitForLine(&passPClerk,id);


	//Set up some convenient variables
	Lock* workLock = &passPClerk.clerkLock[myLineID];
	Condition* workCV = &passPClerk.clerkCV[myLineID];


	//Now the clerk has been woken up and has been told the customer ID
	//Check
	int customerSSN = passportClerkCurrentCustomer[myLineID];
	cout<<"Passport Clerk #"<<id<<" checking on customer #"<<customerSSN<<" and signalling\n";
	passportClerkChecked[customerSSN] = customersWithCompletedApps[customerSSN] && customersWithCompletedPics[customerSSN];
	//And Signal
	workCV->Signal(workLock);
	workCV->Wait(workLock);

	//Now customer is gone
	cout<<"Passport Clerk #"<<id<<" finished with customer #"<<customerSSN<<"\n";
	workLock->Release();

}


void cashierDo(int id){

	//set ID
	int myLineID = id;

	//Wait fot the next cust to signal
	cout<<"Cashier #"<<id<<" about to wait for customer\n";
	waitForLine(&cashier,id);

	//Set up some convenient variables
	Lock* workLock = &cashier.clerkLock[myLineID];
	Condition* workCV = &cashier.clerkCV[myLineID];

	//Now the clerk has been woken up and has been told the customer ID
	//Check
	int customerSSN = cashierCurrentCustomer[myLineID];
	cout<<"Cashier #"<<id<<" checking on customer #"<<customerSSN<<" and signalling\n";
	cashierChecked[customerSSN] = passportClerkChecked[customerSSN];
	//And Signal
	workCV->Signal(workLock);
	workCV->Wait(workLock);

	if(!cashierChecked[customerSSN]){
		//Now customer is gone
		cout<<"Passport Clerk #"<<id<<" finished with customer #"<<customerSSN<<"\n";
		workLock->Release();
	}

	//Now customer has paid, so give passport and mark
	cout<<"Cashier #"<<id<<" marking customer #"<<customerSSN<<" as having gotten passport\n";
	gottenPassport[customerSSN] = true;
	workCV->Signal(workLock);
	workCV->Wait(workLock);

	//Now customer has left
	cout<<"Cashier #"<<id<<" finished with customer #"<<customerSSN<<"\n";
	workLock->Release();

}
