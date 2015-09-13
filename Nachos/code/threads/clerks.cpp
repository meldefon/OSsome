#include "globalVars.h"
#include <iostream>
using namespace std;

void applicationClerk(int id) {
	
	int myLine = id; //the index we pass in will be used as id's for the clerks
					 //which also corresponds to the their line numbers
	while(true) {	
		appClerk->lineLock->Acquire(); //acquire the line lock

		//if there is someone in the bribe line, signal them first
		if(appClerk->bribeLineCount[id] > 0) { 
			appClerk->bribeLineCV[myLine]->Signal(appClerk->lineLock);
			appClerk->clerkState[myLine] = 0; //set state to busy
		}
		else if(appClerk->lineCount[myLine] > 0) { //signal someone in normal line
			appClerk->lineCV[myLine]->Signal(appClerk->lineLock);
			appClerk->clerkState[myLine] = 0; //set state to busy
		} else { //no one is in either line, we must go to sleep
			cout<<"I go to sleep!\n";
		}

		//grab the clerkLock so we can properly signal the waiting customer
		//to avoid a race condition and guarentee the correct order of events
		appClerk->clerkLock[myLine]->Acquire();

		//now we can let go of line lock since we properly acquired the clerk lock
		appClerk->lineLock->Release();

		appClerk->clerkCV[myLine]->Wait(appClerk->clerkLock[myLine]); //wait for customer to signal us
		cout<<"I received social security num!\n"; //ask now for cash
		
		appClerk->clerkCV[myLine]->Signal(appClerk->clerkLock[myLine]); //tell the customer they need cash
		appClerk->clerkCV[myLine]->Wait(appClerk->clerkLock[myLine]); //wait for customer to signal us
		cout<<"I recevied customer's cash!\n";
		
		appClerk->clerkLock[myLine]->Release(); //release the clerk lock since we are done
	}
}