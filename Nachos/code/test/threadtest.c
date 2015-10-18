#include "monitor.h"
#include "customer.c"
#include "clerks.c"
#include "globalVars.h"
    
void initialize(Monitor* m, char* lockName,char* clerkType_, int size) {
        m->lineLock = CreateLock();
        /*lineCV = (int*) malloc(size * sizeof(int));
        clerkLock = (int*) malloc(size * sizeof(int));
        clerkCV = (int*) malloc(size * sizeof(int));
        lineCount = (int*) malloc(size * sizeof(int));
        limboLineCV = CreateCondition();
        bribeLineCount = (int*) malloc(size * sizeof(int));
        bribeLineCV = (int*) malloc(size * sizeof(int));
        senLineCV = (int*) malloc(size * sizeof(int));
        senLineCount = (int*) malloc(size * sizeof(int));
        clerkState = (int*) malloc(size * sizeof(int)); 
        currentCustomer = (int*) malloc(size * sizeof(int));*/
        m->numOfClerks = size;
        m->clerkType = clerkType_;
        m->breakCV = CreateCondition();
        m->numCustomersInLimbo = 0;
        m->cashReceived = 0;

        for(int i = 0; i < size; i++) {
            m->lineCV[i] = CreateCondition();
            m->clerkLock[i] = CreateLock(); 
            m->clerkCV[i] = CreateCondition();    
            m->bribeLineCV[i] = CreateCondition();    
            m->senLineCV[i] = CreateCondition();    

            m->lineCount[i] = 0;
            m->bribeLineCount[i] = 0;
            m->senLineCount[i] = 0;
            m->clerkState[i] = 0;
            m->currentCustomer[i] = -1;
        }
}

void ThreadTest() {
	/*STestSuite();*/

  	int size; /*will be used to take in user input for the sizes of specific variables*/
  	int senatorSize;

	Uprintf("Number of ApplicationClerks = ", 30, 0, 0, 0, 0);
	size = Scanf();

	initialize(&appClerk, "Application Clerk Line Lock","ApplicationClerk", size);

	Uprintf("Number of PictureClerks = ", 26, 0, 0, 0, 0);
	size = Scanf();
	
	initialize(&picClerk, "Picture Clerk Line Lock","PictureClerk", size);

	Uprintf("Number of PassportClerks = ", 27, 0, 0, 0, 0);
	size = Scanf();

	initialize(&passPClerk, "Passport Clerk Line Lock","PassportClerk", size);
	
	Uprintf("Number of Cashiers = ", 21, 0, 0, 0, 0);
	size = Scanf();
	
	initialize(&cashier, "Cashier Line Lock","Cashier", size);
	
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
