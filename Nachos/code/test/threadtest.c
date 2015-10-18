#include "monitor.h"
#include "customer.c"
#include "clerks.c"
#include "globalVars.h"

void ThreadTest() {
	/*STestSuite();*/

  	int size; /*will be used to take in user input for the sizes of specific variables*/
  	int senatorSize;

	Uprintf("Number of ApplicationClerks = ", 30, 0, 0, 0, 0);
	size = Scanf();

	appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", size);

	Uprintf("Number of PictureClerks = ", 26, 0, 0, 0, 0);
	size = Scanf();
	
	picClerk.initialize("Picture Clerk Line Lock","PictureClerk", size);

	Uprintf("Number of PassportClerks = ", 27, 0, 0, 0, 0);
	size = Scanf();

	passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", size);
	
	Uprintf("Number of Cashiers = ", 21, 0, 0, 0, 0);
	size = Scanf();
	
	cashier.initialize("Cashier Line Lock","Cashier", size);
	
	Uprintf("Number of Customers = ", 22, 0, 0, 0, 0);
	size = Scanf();

	int numberOfSenators;
	Uprintf("Number of Senators = ", 21, 0, 0, 0, 0);
	numberOfSenators = Scanf();
	
	size += numberOfSenators;

	/*will hold ints that indicate whether a customer has
	 completed application or pictures using the social security
	 number as an index*/
	
	numCustomersLeft = size;
	/*
	customersWithCompletedApps = (int*) malloc(size * sizeOfInt);
	customersWithCompletedPics = (int*) malloc(size * sizeOfInt);
	passportClerkChecked = (int*) malloc(size * sizeOfInt);
	cashierChecked = (int*) malloc(size * sizeOfInt);
	gottenPassport = (int*) malloc(size * sizeOfInt);
	cashReceived = (int*) malloc(size * sizeOfInt);
	isSenator = (int*) malloc(size * sizeOfInt);
	*/
	bribesEnabled = 1;

	/*Initialize everything*/
	for(int i = 0;i<size;i++) {
		customersWithCompletedApps[i] = 0;
		customersWithCompletedPics[i] = 0;
		passportClerkChecked[i] = 0;
		cashierChecked[i] = 0;
		gottenPassport[i] = 0;
		cashReceived[i] = 0;

		if(i<size-numberOfSenators) {
			isSenator[i]= 0;
		} else {
			isSenator[i]= 1;
		}
	}

	senatorWorking = NULL;
	clerksCanWork = 1;

	/*Instantiate senator lock/CV*/
	senatorLock = CreateLock();
	senatorCV = CreateCondition();
	
	/*will hold currentCust SSN for checking*/
	/*
	appClerkCurrentCustomer = (int*) malloc(appClerk.numOfClerks * sizeOfInt);
	pictureClerkCurrentCustomer = (int*) malloc(picClerk.numOfClerks * sizeOfInt);	
	passportClerkCurrentCustomer = (int*) malloc(passPClerk.numOfClerks * sizeOfInt);	
	cashierCurrentCustomer = (int*) malloc(cashier.numOfClerks * sizeOfInt);	
	*/
	/*initialize all the threads here*/ 
	/*Thread *c;*/

	/*
	for(int i = 0; i < size; i++) {
		c = new Thread("Customer Thread");
		c->Fork((VoidFunctionPtr)customer,i);
	}

	for(int i = 0; i < appClerk.numOfClerks; i++) {
		c = new Thread("AppClerk Thread");
		c->Fork((VoidFunctionPtr)applicationClerk,i);
	}

	for(int i = 0; i < picClerk.numOfClerks; i++) {
		c = new Thread("PicClerk Thread");
		c->Fork((VoidFunctionPtr)pictureClerk,i);
	}


	for(int i = 0; i < passPClerk.numOfClerks; i++) {
		c = new Thread("passPClerk Thread");
		c->Fork((VoidFunctionPtr)passportClerk,i);
	}

	for(int i = 0; i < cashier.numOfClerks; i++) {
		c = new Thread("Cashier Thread");
		c->Fork((VoidFunctionPtr)cashierDo,i);
	}

	c = new Thread("Manager Thread");
	c->Fork((VoidFunctionPtr)managerDo, 0);
	*/
	
	return;
}
