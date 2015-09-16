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
		cout<<clerk->clerkType<<" Clerk #" << myLineID << " signaled someone in the bribe line!\n";
	} else if(clerk->lineCount[myLineID] > 0) { //signal someone in normal line
		clerk->lineCV[myLineID].Signal(clerk->lineLock);
		clerk->clerkState[myLineID] = 0; //set state to busy
		cout<<clerk->clerkType<<" Clerk #" << myLineID  << " signaled someone in the line!\n";
	} else { //no one is in either line, we must go to sleep
		cout<<clerk->clerkType<<" Clerk #" << myLineID  << " is available and waiting!\n";
		clerk->clerkState[myLineID] = 2; //set state to available
	}

	//grab the clerkLock so we can properly signal the waiting customer
	//to avoid a race condition and guarentee the correct order of events
	clerk->clerkLock[myLineID].Acquire();
	cout<<clerk->clerkType<<" Clerk #" << myLineID  << " obtained the "<<clerk->clerkType<<" Clerk Condition Variable Lock!\n";

	//now we can let go of line lock since we properly acquired the clerk lock
	clerk->lineLock->Release();
	cout<<clerk->clerkType<<" Clerk #" << myLineID  << " released the "<<clerk->clerkType<<" Clerk Line Lock!\n";

	clerk->clerkCV[myLineID].Wait(&(clerk->clerkLock[myLineID])); //wait for customer to signal us
	cout<<clerk->clerkType<<" Clerk #" << myLineID  << " received customer's social security number!\n"; //ask now for cash


}

void pictureClerk(int id) {
	
	int myLineID = id; //the index we pass in will be used as id's for the clerks
					 //which also corresponds to the their line numbers
	while(true) {	

		cout<<"Picture Clerk #" << id << " about to wait for customer\n";
		waitForLine(&picClerk, id); 

		picClerk.clerkCV[myLineID].Signal(&(picClerk.clerkLock[myLineID])); //tell the customer they need cash
		picClerk.clerkCV[myLineID].Wait(&(picClerk.clerkLock[myLineID])); //wait for customer to signal us
		cout<<"Picture Clerk #" << id << " received customer's cash and is done!\n"; 
		
		picClerk.clerkLock[myLineID].Release(); //release the clerk lock since we are done
		cout<<"Picture Clerk #" << id << " released the Picture Clerk Condition Variable Lock!\n";
	}
}

void applicationClerk(int id) {
	
	int myLineID = id; //the index we pass in will be used as id's for the clerks
					 //which also corresponds to the their line numbers
	while(true) {	

		cout<<"Application Clerk #" << id << " about to wait for customer\n";
		waitForLine(&appClerk, id); 

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

	while(true) {
		//Wait fot the next cust to signal
		cout << "Passport Clerk #" << id << " about to wait for customer\n";
		waitForLine(&passPClerk, id);


		//Set up some convenient variables
		Lock *workLock = &passPClerk.clerkLock[myLineID];
		Condition *workCV = &passPClerk.clerkCV[myLineID];


		//Now the clerk has been woken up and has been told the customer ID
		//Check
		int customerSSN = passportClerkCurrentCustomer[myLineID];
		cout << "Passport Clerk #" << id << " checking on customer #" << customerSSN << " and signalling\n";
		passportClerkChecked[customerSSN] =
				customersWithCompletedApps[customerSSN] && customersWithCompletedPics[customerSSN];
		//And Signal
		workCV->Signal(workLock);
		workCV->Wait(workLock);

		//Now customer is gone
		cout << "Passport Clerk #" << id << " finished with customer #" << customerSSN << "\n";
		workLock->Release();
	}

}


void cashierDo(int id) {

	//set ID
	int myLineID = id;


	while (true) {
		//Wait fot the next cust to signal
		cout << "Cashier #" << id << " about to wait for customer\n";
		waitForLine(&cashier, id);

		//Set up some convenient variables
		Lock *workLock = &cashier.clerkLock[myLineID];
		Condition *workCV = &cashier.clerkCV[myLineID];

		//Now the clerk has been woken up and has been told the customer ID
		//Check
		int customerSSN = cashierCurrentCustomer[myLineID];
		cout << "Cashier #" << id << " checking on customer #" << customerSSN << " and signalling\n";
		cashierChecked[customerSSN] = passportClerkChecked[customerSSN];
		//And Signal
		workCV->Signal(workLock);
		workCV->Wait(workLock);

		if (!cashierChecked[customerSSN]) {
			//Now customer is gone
			cout << "Passport Clerk #" << id << " finished with customer #" << customerSSN << "\n";
			workLock->Release();
		}

		//Now customer has paid, so give passport and mark
		cout << "Cashier #" << id << " marking customer #" << customerSSN << " as having gotten passport\n";
		gottenPassport[customerSSN] = true;
		workCV->Signal(workLock);
		workCV->Wait(workLock);

		//Now customer has left
		cout << "Cashier #" << id << " finished with customer #" << customerSSN << "\n";
		workLock->Release();
	}

}
