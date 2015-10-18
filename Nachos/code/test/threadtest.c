#include "monitor.h"
#include "customer.c"
#include "clerks.c"
#include "globalVars.h"

void ThreadTest() {
	/*STestSuite();*/

  	int size; /*will be used to take in user input for the sizes of specific variables*/
  	int senatorSize;

	Uprintf("Number of ApplicationClerks = ", 30, 0, 0, 0, 0);
	size = Scanf_syscall();

	appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", size);

	Uprintf("Number of PictureClerks = ", 26, 0, 0, 0, 0);
	size = Scanf_syscall();
	
	picClerk.initialize("Picture Clerk Line Lock","PictureClerk", size);

	Uprintf("Number of PassportClerks = ", 27, 0, 0, 0, 0);
	size = Scanf_syscall();

	passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", size);
	
	Uprintf("Number of Cashiers = ", 21, 0, 0, 0, 0);
	size = Scanf_syscall();
	
	cashier.initialize("Cashier Line Lock","Cashier", size);
	
	Uprintf("Number of Customers = ", 22, 0, 0, 0, 0);
	size = Scanf_syscall();

	int numberOfSenators;
	Uprintf("Number of Senators = ", 21, 0, 0, 0, 0);
	numberOfSenators = Scanf_syscall();
	
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
	Thread *c;

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

void TestSuite() {

	Thread *c;
	int userChoice = 1;
	Uprintf("Test Suite\n\n", 12, 0, 0, 0, 0);

	while(userChoice != 8) {

		Uprintf("1. Customers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time.\n", 116, 0, 0, 0, 0);
		Uprintf("2. Managers only read one from one Clerk's total money received, at a time.\n",76,0,0,0,0);
		Uprintf("3. Customers do not leave until they are given their passport by the Cashier. The Cashier does not start on another customer until they know that the last Customer has left their area.\n",185,0,0,0,0);
 		Uprintf("4. Clerks go on break when they have no one waiting in their line.\n",67,0,0,0,0);
 		Uprintf("5. Managers get Clerks off their break when lines get too long.\n",64,0,0,0,0);
 		Uprintf("6. Total sales never suffers from a race condition.\n",52,0,0,0,0);
 		Uprintf("7. The behavior of Customers is proper when Senators arrive. This is before, during, and after.\n",96,0,0,0,0);
		Uprintf("8. Quit.\n",9,0,0,0,0);
		Uprintf("Pick a test by entering in the test number: ",44,0,0,0,0);
		userChoice = Scanf_syscall();

		if(userChoice == 1) { /*shortest line*/
			
			/*initialize data for clerks, but not the clerk threads*/
			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 3);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 3);
			
			/*initialze globals
			customersWithCompletedApps = new bool[10];
			customersWithCompletedPics = new bool[10];
			isSenator = new bool[10];
			*/
			numCustomersLeft = 10;
			senatorWorking = NULL;
			clerksCanWork = 1;

			/*initialize customer threads*/
			for(int i = 0; i < 10; i++) {
				customersWithCompletedApps[i] = 0;
				customersWithCompletedPics[i] = 0;

				if(i<10){
					isSenator[i]=0;
				}
				else{
					isSenator[i] = 1;
				}

				c = new Thread("Customer Thread");
				c->Fork((VoidFunctionPtr)customer,i);
			}

			senatorLock = CreateLock();
			senatorCV = CreateCondition();
			userChoice = 8;
		} else if(userChoice == 2) {
			/*This test sends 5 customers in plus 1 of each type of clerk. Demonstrates that each type of
			clerk's money is tracked correctly by the manager*/
			cashier.initialize("Cashier Line Lock","Cashier", 1);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 1);
			passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", 1);
			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 1);

			/*initialze globals
			customersWithCompletedApps = new bool[5];
			customersWithCompletedPics = new bool[5];
			passportClerkChecked = new bool[5];
			cashierChecked = new bool[5];
			gottenPassport = new bool[5];
			cashReceived = new int[5];
			cashierCurrentCustomer = new int[1];
			isSenator = new bool[5];
			*/			
			numCustomersLeft = 5;
			numCustomersLeft = 5;
			senatorWorking = NULL;
			clerksCanWork = 1;
			senatorLock = CreateLock();
			senatorCV = CreateCondition();

			c = new Thread("AppClerk Thread");
			c->Fork((VoidFunctionPtr)applicationClerk, 0);

			c = new Thread("PicClerk Thread");
			c->Fork((VoidFunctionPtr)pictureClerk, 0);

			c = new Thread("passportClerk Thread");
			c->Fork((VoidFunctionPtr)passportClerk, 0);

			c = new Thread("Cashier Thread");
			c->Fork((VoidFunctionPtr)cashierDo, 0);

			/*make five customers*/
			for(int i = 0; i < 5; i++) {
				customersWithCompletedApps[i] = 1;
				customersWithCompletedPics[i] = 1;
				passportClerkChecked[i] = 1;
				cashierChecked[i] = 0;
				gottenPassport[i] = 0;
				cashReceived[i] = 0;

				if(i<5){
					isSenator[i]=0;
				}
				else{
					isSenator[i] = 1;
				}

				c = new Thread("Customer Thread");
				c->Fork((VoidFunctionPtr)customer,i);
			}

			c = new Thread("Manager Thread");
			c->Fork((VoidFunctionPtr)managerDo, 0);



			userChoice = 8;
		} else if(userChoice == 3) { /*cashier and customer passport test*/

			cashier.initialize("Cashier Line Lock","Cashier", 1);

			/*initialze globals
			customersWithCompletedApps = new bool[5];
			customersWithCompletedPics = new bool[5];
			passportClerkChecked = new bool[5];
			cashierChecked = new bool[5];
			gottenPassport = new bool[5];
			cashReceived = new int[5];
			cashierCurrentCustomer = new int[1];
			isSenator = new bool[5];
			*/
			numCustomersLeft = 5;
			senatorWorking = NULL;
			clerksCanWork = 1;
			senatorLock = CreateLock();
			senatorCV = CreateCondition();

			/*make five customers*/
			for(int i = 0; i < 5; i++) {
				customersWithCompletedApps[i] = 1;
				customersWithCompletedPics[i] = 1;
				passportClerkChecked[i] = 1;
				cashierChecked[i] = 0;
				gottenPassport[i] = 0;
				cashReceived[i] = 0;

				if(i<5){
					isSenator[i]=0;
				}
				else{
					isSenator[i] = 1;
				}

				c = new Thread("Customer Thread");
				c->Fork((VoidFunctionPtr)customer,i);
			}

			/*make one cashier*/
			c = new Thread("Cashier Thread");
			c->Fork((VoidFunctionPtr)cashierDo,0);	
			userChoice = 8;	
		} else if(userChoice == 4) { /*clerks go on break when no one is in line*/

			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 1);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 1);
			passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", 1);
			cashier.initialize("Cashier Line Lock","Cashier", 1);

			/*isSenator = new bool[1];*/
			isSenator[0] = 0;
			numCustomersLeft = 1;
			senatorWorking = NULL;
			clerksCanWork = 1;
			senatorLock = CreateLock();
			senatorCV = CreateCondition();

			for(int i = 0; i < appClerk.numOfClerks; i++) {
				c = new Thread("AppClerk Thread");
				c->Fork((VoidFunctionPtr)applicationClerk, 0);
			}

			for(int i = 0; i < picClerk.numOfClerks; i++) {
				c = new Thread("PicClerk Thread");
				c->Fork((VoidFunctionPtr)pictureClerk, 0);
			}

			for(int i = 0; i < passPClerk.numOfClerks; i++) {
				c = new Thread("passPClerk Thread");
				c->Fork((VoidFunctionPtr)passportClerk, 0);
			}

			for(int i = 0; i < cashier.numOfClerks; i++) {
				c = new Thread("Cashier Thread");
				c->Fork((VoidFunctionPtr)cashierDo, 0);
			}

			userChoice = 8;
		} else if(userChoice == 5) { /*manager gets clerk off break*/
			
			cashier.initialize("Cashier Line Lock","Cashier", 1);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 1);
			passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", 1);
			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 1);

			/*initialze globals
			customersWithCompletedApps = new bool[5];
			customersWithCompletedPics = new bool[5];
			passportClerkChecked = new bool[5];
			cashierChecked = new bool[5];
			gottenPassport = new bool[5];
			cashReceived = new int[5];
			cashierCurrentCustomer = new int[1];
			isSenator = new bool[5];
			*/			
			numCustomersLeft = 5;
			numCustomersLeft = 5;
			senatorWorking = NULL;
			clerksCanWork = 1;
			senatorLock = CreateLock();
			senatorCV = CreateCondition();

			/*make one cashier*/
			c = new Thread("Cashier Thread");
			c->Fork((VoidFunctionPtr)cashierDo, 0);
			
			/*make five customers*/
			for(int i = 0; i < 5; i++) {
				customersWithCompletedApps[i] = 1;
				customersWithCompletedPics[i] = 1;
				passportClerkChecked[i] = 1;
				cashierChecked[i] = 0;
				gottenPassport[i] = 0;
				cashReceived[i] = 0;
				
				if(i<5){
					isSenator[i]=0;
				}
				else{
					isSenator[i] = 1;
				}
				
				c = new Thread("Customer Thread");
				c->Fork((VoidFunctionPtr)customer,i);
			}

			c = new Thread("Manager Thread");
			c->Fork((VoidFunctionPtr)managerDo, 0);
			userChoice = 8;

		} else if(userChoice == 6) {
			/*This test sends 4 customers in plus 1 senator*/
			cashier.initialize("Cashier Line Lock","Cashier", 1);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 1);
			passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", 1);
			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 1);

			int numCustsForTest = 10;
			bribesEnabled = 0;
			/*initialze globals
			customersWithCompletedApps = new bool[numCustsForTest];
			customersWithCompletedPics = new bool[numCustsForTest];
			passportClerkChecked = new bool[numCustsForTest];
			cashierChecked = new bool[numCustsForTest];
			gottenPassport = new bool[numCustsForTest];
			cashReceived = new int[numCustsForTest];
			cashierCurrentCustomer = new int[1];
			isSenator = new bool[numCustsForTest];
			*/			
			numCustomersLeft = numCustsForTest;
			numCustomersLeft = numCustsForTest;
			senatorWorking = NULL;
			clerksCanWork = 1;
			senatorLock = CreateLock();
			senatorCV = CreateCondition();

			c = new Thread("AppClerk Thread");
			c->Fork((VoidFunctionPtr)applicationClerk, 0);

			c = new Thread("PicClerk Thread");
			c->Fork((VoidFunctionPtr)pictureClerk, 0);

			c = new Thread("passportClerk Thread");
			c->Fork((VoidFunctionPtr)passportClerk, 0);

			c = new Thread("Cashier Thread");
			c->Fork((VoidFunctionPtr)cashierDo, 0);

			/*make five customers*/
			for(int i = 0; i < numCustsForTest; i++) {
				customersWithCompletedApps[i] = 1;
				customersWithCompletedPics[i] = 1;
				passportClerkChecked[i] = 1;
				cashierChecked[i] = 0;
				gottenPassport[i] = 0;
				cashReceived[i] = 0;

				if(i<numCustsForTest){
					isSenator[i]=0;
				}
				else{
					isSenator[i] = 1;
				}

				c = new Thread("Customer Thread");
				c->Fork((VoidFunctionPtr)customer,i);
			}

			c = new Thread("Manager Thread");
			c->Fork((VoidFunctionPtr)managerDo, 0);
			userChoice = 8;


		} else if(userChoice == 7) {
			/*This test sends 4 customers in plus 1 senator*/
			cashier.initialize("Cashier Line Lock","Cashier", 1);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 1);
			passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", 1);
			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 1);

			/*initialze globals
			customersWithCompletedApps = new bool[5];
			customersWithCompletedPics = new bool[5];
			passportClerkChecked = new bool[5];
			cashierChecked = new bool[5];
			gottenPassport = new bool[5];
			cashReceived = new int[5];
			cashierCurrentCustomer = new int[1];
			isSenator = new bool[5];
			*/
			numCustomersLeft = 5;
			senatorWorking = NULL;
			clerksCanWork = 1;
			senatorLock = CreateLock();
			senatorCV = CreateCondition();

			c = new Thread("AppClerk Thread");
			c->Fork((VoidFunctionPtr)applicationClerk, 0);

			c = new Thread("PicClerk Thread");
			c->Fork((VoidFunctionPtr)pictureClerk, 0);

			c = new Thread("passportClerk Thread");
			c->Fork((VoidFunctionPtr)passportClerk, 0);

			c = new Thread("Cashier Thread");
			c->Fork((VoidFunctionPtr)cashierDo, 0);

			/*make five customers*/
			for(int i = 0; i < 5; i++) {
				customersWithCompletedApps[i] = 1;
				customersWithCompletedPics[i] = 1;
				passportClerkChecked[i] = 1;
				cashierChecked[i] = 0;
				gottenPassport[i] = 0;
				cashReceived[i] = 0;

				if(i<4){
					isSenator[i]=0;
				}
				else{
					isSenator[i] = 1;
				}

				c = new Thread("Customer Thread");
				c->Fork((VoidFunctionPtr)customer,i);
			}

			c = new Thread("Manager Thread");
			c->Fork((VoidFunctionPtr)managerDo, 0);
			userChoice = 8;


			userChoice = 8;
		}
	}
}
