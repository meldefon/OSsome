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

int numCustomersLeft;

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
		bool didBribe = false;

		int wantToBribe = rand() % 10; // random choice about whether to bribe
		if(strcmp(clerk->clerkType,"Cashier")){
			wantToBribe = 1;
		}
		//bool wantToBribe = socialSecurityNum==4;
		if(wantToBribe==0 && *cash>100){
			*cash-=500;
			lineCount = clerk->bribeLineCount;
			lineCV = clerk->bribeLineCV;
			didBribe = true;
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
		
		//pick the shortest clerk line
		//cout<<"Customer #" << socialSecurityNum << " has acquired the "<<clerk->lineLock->getName()<<". Bribe:"<<wantToBribe<<"\n";

		int myLine = -1;

		while(myLine == -1) { //if we haven't found a line, we need to loop back again

			int lineSize = 777;

			for(int i = 0; i < clerk->numOfClerks; i++) {
				
				if(clerk->clerkState[i] == 2) { //or the clerk is available
					myLine = i;
					lineSize = lineCount[i];
					break; //leave since we found the right clerk
				}
				//Right now, customer picks shortest line regardless of break status
				else if(lineCount[i] < lineSize/* && clerk->clerkState[i] != 1*/) { //if the line we are in is the
					// shortest and the clerk is not on break, consider that line
					myLine = i;
					lineSize = lineCount[i];
				}
			}
			
			//if all the clerks on are on break
			if(myLine == -1) {
				cout<<"Customer #" << socialSecurityNum << " has entered "<<clerk->clerkType<<" Limbo Line.\n";
				clerk->numCustomersInLimbo++;
				clerk->limboLineCV->Wait(clerk->lineLock); //wait to be signaled by manager to wake up from limbo line
			}
		}
		
		if(clerk->clerkState[myLine] == 0 || clerk->clerkState[myLine]==1) { //if the clerk is busy with another customer, we must wait, else just
			// bypass this and go straight to transaction
			if(!didBribe)
				cout<<"Customer #" << socialSecurityNum << " has gotten in regular line for "<<clerk->clerkType<<" #" << myLine << ".\n";
			else
				cout<<"Customer #" << socialSecurityNum << " has gotten in bribe line for "<<clerk->clerkType<<" #" << myLine << ".\n";
			
			lineCount[myLine]++; //get in line
			lineCV[myLine].Wait(clerk->lineLock); //wait until we are signaled by AppClerk
			lineCount[myLine]--; //get out of line and move to the counter
		}
			
		clerk->clerkState[myLine] = 0;   //set the clerk status to busy
		//cout<<"Customer #" << socialSecurityNum << " is being served by "<<clerk->clerkType<<" Clerk #" << myLine <<"!\n";
		
		clerk->lineLock->Release(); //release the lock since we now can begin to conduct the transaction 
									//which is only relavent to the two threads of customer and clerk
		//cout<<"Customer #" << socialSecurityNum << " has released the "<<clerk->clerkType<<" Clerk Line Lock!\n";

	return myLine;
}

void doAppClerkStuff(int socialSecurityNum, int* cash) {

	int myLine = getInLine(&appClerk,socialSecurityNum, cash);

	//now we must obtain the lock from the AppClerk which went to wait state once he was avaiable and 
	//waiting for a customer to signal him
	appClerk.clerkLock[myLine].Acquire();
	//cout<<"Customer #" << socialSecurityNum << " has acquired the Application Clerk #" << myLine << " Condition Variable Lock!\n";
	
	appClerk.currentCustomer[myLine] = socialSecurityNum; //which customer is currently being served
	cout<<"Customer #" <<socialSecurityNum << " has given SSN "<< socialSecurityNum << " to ApplicationClerk #" << myLine << ".\n";	

	appClerk.clerkCV[myLine].Signal(&(appClerk.clerkLock[myLine])); //signal the AppClerk that we have given him the social security number
	appClerk.clerkCV[myLine].Wait(&(appClerk.clerkLock[myLine])); 	//wait for the clerk to confirm

	appClerk.clerkCV[myLine].Signal(&(appClerk.clerkLock[myLine])); //signal the clerk

	appClerk.clerkLock[myLine].Release(); //let go of the lock
	//cout<<"Customer #" << socialSecurityNum << " has released the Application Clerk #" << myLine << " Condition Variable Lock!\n";
}

int doPicClerkStuff(int socialSecurityNum, int* cash) {

	int myLine = getInLine(&picClerk,socialSecurityNum,cash);
	int choice = 0;

	//now we must obtain the lock from the PicClerk which went to wait state once he was avaiable and 
	//waiting for a customer to signal him
	picClerk.clerkLock[myLine].Acquire();
	picClerk.currentCustomer[myLine] = socialSecurityNum; //which customer is currently being served
	cout<<"Customer #" <<socialSecurityNum << " has given SSN "<< socialSecurityNum << " to PictureClerk #" << myLine << ".\n";	

	//cout<<"Customer #" << socialSecurityNum << " has acquired the Picture Clerk #" << myLine << " Condition Variable Lock!\n";

	//signal the PicClerk that we have given him the social security number
	picClerk.clerkCV[myLine].Signal(&(picClerk.clerkLock[myLine]));
	//wait for the clerk to confirm then we decide if we like the photo or not
	picClerk.clerkCV[myLine].Wait(&(picClerk.clerkLock[myLine]));
	
	int probablity = rand() % 10; //generate a random # from 0 - 9, if less than or equal to 4, we retake photo

	if(probablity <= 4) { //if we disliked the photo
			cout<<"Customer #" << socialSecurityNum << " does not like their picture from Picture Clerk #" << myLine << ".\n";
			customersWithCompletedPics[socialSecurityNum] = false;
			choice = 1;
	} else {
			cout<<"Customer #" << socialSecurityNum << " likes their picture from Picture Clerk #" << myLine << ".\n";
			customersWithCompletedPics[socialSecurityNum] = true;
			choice = 0;		
	}

	picClerk.clerkCV[myLine].Signal(&(picClerk.clerkLock[myLine])); //signal the clerk
	picClerk.clerkLock[myLine].Release(); //let go of the lock
	
	return choice;
	//cout<<"Customer #" << socialSecurityNum << " has released the Picture Clerk #" << myLine << " Condition Variable Lock!\n";
}

void doPassportClerkStuff(int socialSecurityNum,int*cash){

	int mySSN = socialSecurityNum;

	//First get in line with a generic method
	//cout<<"Customer #"<<socialSecurityNum<<" getting in passport Line\n";
	int myLine = getInLine(&passPClerk,socialSecurityNum,cash);

	//Enter interaction monitor with passport clerk
	Lock* workLock = &passPClerk.clerkLock[myLine];
	Condition* workCV = &passPClerk.clerkCV[myLine];
	workLock->Acquire();

	//Tell Clerk CV, then wait
	cout<<"Customer #"<<socialSecurityNum<<" has given SSN "<<socialSecurityNum<<" to PassportClerk #"<<myLine<<"\n";
	tellPassportClerkSSN(mySSN,myLine);
	workCV->Signal(workLock);
	workCV->Wait(workLock);

	//Now leave
	//cout<<"Customer #"<<socialSecurityNum<<" leaving passportClerk #"<<myLine<<"\n";
	workCV->Signal(workLock);
	workLock->Release();

	//Decide weather to self-punish
	bool myPassportChecked = passportClerkChecked[mySSN];
	if(!myPassportChecked) {
		cout<<"Customer #"<<socialSecurityNum<<" has gone to PassportClerk #"<<myLine<<" too soon. "<<
				"They are going to the back of the line.\n";
		punish(punishTime);
	}
	return;

}

void doCashierStuff(int mySSN, int* cash){

	int socialSecurityNum = mySSN;

	//First get in line with a generic method
	//cout<<"Customer #"<<socialSecurityNum<<" getting in cashier line\n";
	int myLine = getInLine(&cashier,socialSecurityNum,cash);

	//Enter interaction monitor with passport clerk
	Lock* workLock = &cashier.clerkLock[myLine];
	Condition* workCV = &cashier.clerkCV[myLine];
	workLock->Acquire();

	//Tell Clerk CV, then wait
	cout<<"Customer #"<<socialSecurityNum<<" has given SSN "<<socialSecurityNum<<" to Cashier #"<<myLine<<"\n";
	tellCashierSSN(mySSN,myLine);
	workCV->Signal(workLock);
	workCV->Wait(workLock);

	//Decide weather to self-punish
	bool readyToPay = cashierChecked[mySSN];
	if(!readyToPay) {
		//Release, punish, and leave
		cout<<"Customer #"<<socialSecurityNum<<" has gone to Cashier #"<<myLine<<" too soon. "<<
				"They are going to the back of the line.\n";
		workCV->Signal(workLock);
		workLock->Release();
		punish(punishTime);
		return;
	}

	//Now you can pay
	cout<<"Customer #"<<socialSecurityNum<<" has given Cashier #"<<myLine<<" $100\n";
	payCashier(mySSN,cash);
	workCV->Signal(workLock);
	workCV->Wait(workLock);

	//Now you've been woken up because you have the passport, so leave
	//cout<<"Customer #"<<socialSecurityNum<<" got passport and is leaving cashier #"<<myLine<<"\n";
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
		else if(!passportClerkChecked[socialSecurityNum]){
			doPassportClerkStuff(socialSecurityNum,&cash);
			continue;
		}
		//DO the cashier stuff
		else if(!cashierChecked[socialSecurityNum]){
			doCashierStuff(socialSecurityNum,&cash);
			continue;
		}
		else{
			notCompleted = false;
		}
	}

	cout<<"Customer #"<<socialSecurityNum<<" is leaving the Passport Office.\n";
	numCustomersLeft-=1;
}
