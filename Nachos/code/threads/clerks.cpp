#include "globalVars.h"
#include "synch.h"
#include <iostream>
#include <string.h>
using namespace std;


/*int*/bool waitForLine(Monitor* clerk,int myLineID, bool firstTime/*int firstTime*/){
	clerk->lineLock->Acquire(); //acquire line lock
	//Acquire(clerk->lineLock);
	bool ifBribed = false;
	//int sizeOfInt = sizeof(int);
	//int ifBribed = 0;

	//cout<<myLineID<<"Waiting for line\n";
	while(true) {

		if(senatorWorking!=NULL){

			if( clerk->senLineCount[myLineID]>0/* *(clerk->senLineCount + (myLineID * sizeOfInt)) > 0 */) {
				clerk->senLineCV[myLineID].Signal(clerk->lineLock);
				//Signal(*(clerk->senLineCV + (myLineID * sizeof(int))), clerk->lineLock);
				clerk->clerkState[myLineID] = 0;
				//*(clerk->clerkState + (myLineID * sizeOfInt)) = 0;
				//cout<<"Serving senator\n";
				break;
			}
			else{
				clerk->clerkState[myLineID] = 1; //set state to break
				//*(clerk->clerkState + (myLineID * sizeOfInt)) = 1;
				clerk->breakCV->Wait(clerk->lineLock);
				//Wait(clerk->breakCV, clerk->lineLock);
				//cout<<"Going on senator break\n";
				continue;
			}
		}
		//if there is someone in the bribe line, signal them first
		else if (clerk->bribeLineCount[myLineID] > 0/* *(clerk->bribeLineCount + (myLineID * sizeOfInt)) > 0 */) {
			clerk->bribeLineCV[myLineID].Signal(clerk->lineLock);
			//Signal(*(clerk->bribeLineCV + (myLineID * sizeOfInt)), clerk->lineLock);
			ifBribed = true;
			//ifBribed = 1;
			clerk->clerkState[myLineID] = 0; //set state to busy
			//*(clerk->clerkState + (myLineID * sizeOfInt)) = 0;
			cout << clerk->clerkType << " #" << myLineID << " has signalled a Customer to come to their counter.\n";
			break;
		} else if (clerk->lineCount[myLineID] > 0 /* *(clerk->lineCount + (myLineID * sizeOfInt)) > 0 */) { //signal someone in normal line
			clerk->lineCV[myLineID].Signal(clerk->lineLock);
			//Signal(*(clerk->lineCV + (myLineID * sizeOfInt)), clerk->lineLock);
			clerk->clerkState[myLineID] = 0; //set state to busy
			//*(clerk->clerkState + (myLineID * sizeOfInt)) = 0;
			cout << clerk->clerkType << " #" << myLineID << " has signalled a Customer to come to their counter.\n";
			break;
		} else { //no one is in either line, we must go to sleep

			firstTime = false;
			//firstTime = 0;
			if (!firstTime /*firstTime == 0*/) {
				cout << clerk->clerkType << " #" << myLineID << " is going on break.\n";
				clerk->clerkState[myLineID] = 1; //set state to break
				//*(clerk->clerkState + (myLineID * sizeOfInt)) = 1;
				clerk->breakCV->Wait(clerk->lineLock); //wait for the manager to signal
				//Wait(clerk->breakCV, clerk->lineLock);
				cout << clerk->clerkType << " #" << myLineID << " is coming off break.\n";
			}

			clerk->clerkState[myLineID] = 0; //set state to available
			//*(clerk->clerkState + (myLineID * sizeOfInt)) = 0;
			//return;
			continue;
		}
	}

	//grab the clerkLock so we can properly signal the waiting customer
	//to avoid a race condition and guarentee the correct order of events
	clerk->clerkLock[myLineID].Acquire();
	//Acquire(*(clerk->clerkLock + (myLineID * sizeOfInt)));
	//cout<<clerk->clerkType<<" Clerk #" << myLineID  << " obtained the "<<clerk->clerkType<<" Clerk Condition Variable Lock!\n";

	//now we can let go of line lock since we properly acquired the clerk lock
	clerk->lineLock->Release();
	//Release(clerk->lineLock);
	//cout<<clerk->clerkType<<" Clerk #" << myLineID  << " released the "<<clerk->clerkType<<" Clerk Line Lock!\n";

	clerk->clerkCV[myLineID].Wait(&(clerk->clerkLock[myLineID])); //wait for customer to signal us
	//Wait(*(clerk->clerkCV + (myLineID * sizeOfInt)), *(clerk->clerkLock + (myLineID * sizeOfInt)));
	return ifBribed;
	//cout<<clerk->clerkType<<" Clerk #" << myLineID  << " received customer's social security number!\n"; //ask now for cash
}

void pictureClerk(int id) {
	
	int myLineID = id; //the index we pass in will be used as id's for the clerks
	bool firstTime = true;
	//int firstTime = 1;
	bool ifBribed;
	//int ifBribed;
	//int sizeOfInt = sizeof(int);

	while(true) {	
		//cout<<"Picture Clerk #" << id << " about to wait for customer\n";
		ifBribed = waitForLine(&picClerk, id, firstTime);

		cout<<"PictureClerk #" << id << " has received SSN " << picClerk.currentCustomer[id] << " from Customer #" << picClerk.currentCustomer[id] << ".\n";
		cout<<"PictureClerk #" << id << " has taken a picture of Customer #" << picClerk.currentCustomer[id] << ".\n";

		//int picClerkCV = *(picClerk.clerkCV + (myLineID * sizeOfInt));
		//int picClerkLock = *(picClerk.clerkLock + (myLineID * sizeOfInt));
		picClerk.clerkCV[myLineID].Signal(&(picClerk.clerkLock[myLineID])); //tell the customer the picture is completed
		//Signal(picClerkCV, picClerkLock);
		picClerk.clerkCV[myLineID].Wait(&(picClerk.clerkLock[myLineID])); //wait for customer to decide if they like the photo
		//Wait(picClerkCV, picClerkLock);

		//int currentPicClerkCust = *(picClerk.currentCustomer + (id * sizeOfInt));

		if(customersWithCompletedPics[picClerk.currentCustomer[id]] == true 		/* *(customersWithCompletedPics + (currentPicClerkCust + sizeOfInt)) == 1 */) {
			cout<<"PictureClerk #" << id << " has been told that Customer #" << picClerk.currentCustomer[id] << " does like their picture.\n";
			
			int yieldCalls = (rand() % 80) + 21; //generate # from 20-100

			for(int i = 0; i < yieldCalls; i++) { //delay in filing the picture
				currentThread->Yield();
			}

		} else {
			cout<<"PictureClerk #" << id << " has been told that Customer #" << picClerk.currentCustomer[id] << " does not like their picture.\n";
		}

		if(ifBribed /*ifBribed == 1*/) {
			cout<<"PictureClerk #" << id << " has received $500 from Customer #" << picClerk.currentCustomer[id] << ".\n";
			picClerk.cashReceived+=500;
		}

		firstTime = false;
		//firstTime = 0;
		picClerk.clerkLock[myLineID].Release(); //release the clerk lock since we are done
		//Release(picClerkLock);
		//cout<<"Picture Clerk #" << id << " released the Picture Clerk Condition Variable Lock!\n";
	}
}

void applicationClerk(int id) {
	
	int myLineID = id; //the index we pass in will be used as id's for the clerks
	bool firstTime = true;
	//int firstTime = 1;
	bool ifBribed;
	//int ifBribed;
	//int sizeOfInt = sizeof(int);

	while(true) {	
		//cout<<"Application Clerk #" << id << " about to wait for customer\n";
		waitForLine(&appClerk, id, firstTime);


		cout<<"ApplicationClerk #" << id << " has received SSN " << appClerk.currentCustomer[id] << " from Customer #" << appClerk.currentCustomer[id] << ".\n";
		
		//int appClerkCV = *(appClerk.clerkCV + (myLineID * sizeOfInt));
		//int appClerkLock = *(appClerk.clerkLock + (myLineID * sizeOfInt));
		//int currentAppClerkCust = *(appClerk.currentCustomer + (id * sizeOfInt));

		//input the socialSecurityNum into the completed applications
		customersWithCompletedApps[appClerk.currentCustomer[id]] = true;
		//*(customersWithCompletedApps + (currentAppClerkCust * sizeOfInt)) = 1;

		int yieldCalls = (rand() % 80) + 21; //generate # from 20-100

		for(int i = 0; i < yieldCalls; i++) { //delay in filing the application
			currentThread->Yield();
		}

		appClerk.clerkCV[myLineID].Signal(&(appClerk.clerkLock[myLineID])); //tell the customer their application is done
		//Signal(appClerkCV, appClerkLock);
		appClerk.clerkCV[myLineID].Wait(&(appClerk.clerkLock[myLineID])); //wait for customer to leave us
		//Wait(appClerkCV, appClerkLock);

		cout<<"ApplicationClerk #" << id << " has recorded a completed application for Customer #" << appClerk.currentCustomer[id] << ".\n";
		
		if(ifBribed /*ifBribed == 1*/) {
			cout<<"ApplicationClerk #" << id << " has received $500 from Customer #" << appClerk.currentCustomer[id] << ".\n";
			appClerk.cashReceived+=500;
		}

		firstTime = false;
		//firstTime = 0;
		appClerk.clerkLock[myLineID].Release(); //release the clerk lock since we are done
		//Release(appClerkLock);
		//cout<<"Application Clerk #" << id << " released the Application Clerk Condition Variable Lock!\n";
	}
}

void passportClerk(int id) {

	int myLineID = id; 	//set ID
	bool firstTime = true;
	//int firstTime = 1;
	bool ifBribed;
	//int ifBribed;
	//int sizeOfInt = sizeof(int);

	while(true) {
		//Wait fot the next cust to signal
		ifBribed = waitForLine(&passPClerk, id, firstTime);


		//Set up some convenient variables
		Lock *workLock = &passPClerk.clerkLock[myLineID];
		//int workLock = *(passPClerk.clerkLock + (myLineID * sizeOfInt));
		Condition *workCV = &passPClerk.clerkCV[myLineID];
		//int workCV = *(passPClerk.clerkCV + (myLineID * sizeOfInt));

		//Now the clerk has been woken up and has been told the customer ID
		//Check
		int customerSSN = passportClerkCurrentCustomer[myLineID];
		//int customerSSN = *(passportClerkCurrentCustomer + (myLineID * sizeOfInt));

		cout<<"PassportClerk #" << id << " has received SSN " << customerSSN << " from Customer #" << customerSSN << ".\n";
		passportClerkChecked[customerSSN] =
				customersWithCompletedApps[customerSSN] && customersWithCompletedPics[customerSSN];
		//*(passportClerkChecked + (customerSSN * sizeOfInt)) = *(customersWithCompletedApps + (customerSSN * sizeOfInt)) && *(customersWithCompletedPics + (customerSSN * sizeOfInt));
		if(!passportClerkChecked[customerSSN]/* *(passportClerkChecked (customerSSN + sizeOfInt)) == 0 */){
			cout<<"PassportClerk #"<<id<<" has determined that Customer #"<<customerSSN<<
					" does not have both their application and picture completed\n";
		}
		else{
			cout<<"PassportClerk #"<<id<<" has determined that Customer #"<<customerSSN<<
			" has both their application and picture completed\n";
		}
		//And Signal
		workCV->Signal(workLock);
		//Signal(workCV, workLock);
		workCV->Wait(workLock);
		//Wait(workCV, workLock);

		//Now customer is gone
		if(passportClerkChecked[customerSSN]/* *(passportClerkChecked + (customerSSN * sizeOfInt)) == 1 */) {
			cout << "PassportClerk #" << id << " has recorded Customer #" << customerSSN <<
			" passport documentation\n";
		}

		if(ifBribed /*ifBribed == 1*/) {
			cout<<"PassportClerk #" << id << " has received $500 from Customer #" << customerSSN << ".\n";
			passPClerk.cashReceived+=500;
		}

		firstTime = false;
		//firstTime = 0;
		workLock->Release();
		//Release(workLock);
	}
}

void cashierDo(int id) {

	int myLineID = id; 	//set ID
	bool firstTime = true;
	//int firstTime = 1;
	bool ifBribed;
	//int ifBribed;
	//int sizeOfInt = sizeof(int);

	while (true) {
		//Wait fot the next customer to signal
		//cout << "Cashier #" << id << " about to wait for customer\n";
		waitForLine(&cashier, id, firstTime);


		//Set up some convenient variables
		Lock *workLock = &cashier.clerkLock[myLineID];
		//int workLock = *(cashier.clerkLock + (myLineID * sizeOfInt));
		Condition *workCV = &cashier.clerkCV[myLineID];
		//int workCV = *(cashier.clerkCV + (myLineID * sizeOfInt));

		//Now the clerk has been woken up and has been told the customer ID
		//Check
		int customerSSN = cashierCurrentCustomer[myLineID];
		//int customerSSN = *(cashierCurrentCustomer + (myLineID * sizeOfInt));
		
		cout<<"Cashier #" << id << " has received SSN " << customerSSN << " from Customer #" << customerSSN << ".\n";
		cashierChecked[customerSSN] = passportClerkChecked[customerSSN];
		//*(cashierChecked + (customerSSN * sizeOfInt)) = *(passportClerkChecked + (customerSSN * sizeOfInt));
		//And Signal
		workCV->Signal(workLock);
		//Signal(workCV, workLock);
		workCV->Wait(workLock);
		//Wait(workCV, workLock);

		if (!cashierChecked[customerSSN]/* *(cashierChecked + (customerSSN * sizeOfInt)) == 0 */) {
			//Now customer is gone
			cout<<"Cashier #"<<id<<" has recieved $100 from Customer #"<<customerSSN<<
					" before certification. They are to go to the back of the line\n";
			workLock->Release();
			//Release(workLock);
			return;
		}

		//Now customer has paid, so give passport and mark
		cout<<"Cashier #"<<id<<" has verified that Customer #"<<customerSSN<<" has been certified by a PassportClerk\n";
		cout<<"Cashier #"<<id<<" has recieved $100 from Customer #"<<customerSSN<<" after certification\n";
		cout << "Cashier #" << id << " provided Customer #" << customerSSN << " with their completed passport\n";
		cashier.cashReceived+=100;
		gottenPassport[customerSSN] = true;
		//*(gottenPassport + (customerSSN * sizeOfInt)) = 1;
		workCV->Signal(workLock);
		//Signal(workCV, workLock);
		workCV->Wait(workLock);
		//Wait(workCV, workLock);

		//Now customer has left
		cout << "Cashier #" << id << " has recorded that Customer #" << customerSSN <<
				" has been given their completed passport\n";
		firstTime = false;
		//firstTime = 0;
		workLock->Release();
		//Release(workLock);
	}

	//Exit(0);

}

void checkForClerkOnBreak(Monitor *clerk) {

	clerk->lineLock->Acquire(); //acquire the line lock 
	//Acquire(clerk->lineLock);
	bool clerksOnBreak = false;
	//int clerksOnBreak = 0;
	//int sizeOfInt = sizeof(int);

	//check if there are any clerks on break
	for(int i = 0; i <clerk->numOfClerks; i++) {
		if(clerk->clerkState[i] == 1/* *(clerk->clerkState + (i * sizeOfInt)) == 1 */) {
			clerksOnBreak = true;
			//clerksOnBreak = 1;
			break;
		}
	}

	int lineThreshold = 0;
	int senLineThreshold = 0;
	if(clerksOnBreak /*clerksOnBreak == 1*/) {
		//check if there is a particular line with more than 3 customers waiting
		for(int i = 0; i < clerk->numOfClerks; i++) {
			if(clerk->senLineCount[0] > senLineThreshold || (senatorWorking==NULL && (clerk->lineCount[i] > lineThreshold ||
					clerk->bribeLineCount[i] > lineThreshold))) {
			//if(*(clerk->senLineCount) > senLineThreshold || (senatorWorking==NULL) && (*(clerk->lineCount +(i * sizeOfInt)) > lineThreshold || *(clerk->bribeLineCount +(i * sizeOfInt)) > lineThreshold))
				/*if(clerk->numCustomersInLimbo != 0) {
					clerk->limboLineCV->Broadcast(clerk->lineLock); //wake up all sleeping customers
					clerk->numCustomersInLimbo = 0;
				}*/

				for(int j = 0; j < clerk->numOfClerks; j++) {

					if(clerk->clerkState[j] == 1 /* *(clerk->clerkState + (j * sizeOfInt)) == 1 */) { //if a clerk is on break, wake them up
						clerk->breakCV->Signal(clerk->lineLock);
						//Signal(clerk->breakCV, clerk->lineLock);

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
	//Release(clerk->lineLock);
}

void managerDo(int id) {

	int myID = id; 	//set ID

	while (numCustomersLeft>0) {
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
		for(int i = 0; i < 20; i++) {
			//cout<<"Manager looping\n";
			currentThread->Yield();
		}
	}	
}












