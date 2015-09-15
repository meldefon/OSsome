#include "globalVars.h"
#include <iostream>
using namespace std;

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