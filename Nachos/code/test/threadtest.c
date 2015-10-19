#include "monitor.h"
#include "customer.c"
#include "clerks.c"
#include "globalVars.h"
#define NULL 0

void initialize(struct Monitor *m, char* lockName,int clerkType_, int size) {
        int i;

        m->lineLock = CreateLock();
   		m->newClerkId = 0;
    	m->newClerkIdLock = CreateLock();        
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

        for(i = 0; i < size; i++) {
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

int main() {
	/*STestSuite();*/

  	int size; /*will be used to take in user input for the sizes of specific variables*/
  	int senatorSize;
	int i;
    int j;
    int k;
    int l;
    int m;
    int n;
	int numberOfSenators;	

	Uprintf("Number of ApplicationClerks = ", 30, 0, 0, 0, 0);
	size = Scanf();

	initialize(&appClerk, "Application Clerk Line Lock",0, size);

	Uprintf("Number of PictureClerks = ", 26, 0, 0, 0, 0);
	size = Scanf();
	
	initialize(&picClerk, "Picture Clerk Line Lock",1, size);

	Uprintf("Number of PassportClerks = ", 27, 0, 0, 0, 0);
	size = Scanf();

	initialize(&passPClerk, "Passport Clerk Line Lock",2, size);
	
	Uprintf("Number of Cashiers = ", 21, 0, 0, 0, 0);
	size = Scanf();
	
	initialize(&cashier, "Cashier Line Lock",3, size);
	
	Uprintf("Number of Customers = ", 22, 0, 0, 0, 0);
	size = Scanf();

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
	newCustomerId = 0;
	newCustomerIdLock = CreateLock();

	/*Initialize everything*/
	for(i = 0;i<size;i++) {
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

	for(j = 0; j < size; j++) {
		Fork(customer);
	}

	for(k = 0; k < appClerk.numOfClerks; k++) {
		Fork(applicationClerk);
	}

	for(l = 0; l < picClerk.numOfClerks; l++) {
		Fork(pictureClerk);
	}

	for(m = 0; m < passPClerk.numOfClerks; m++) {
		Fork(passportClerk);
	}

	for(n = 0; n < cashier.numOfClerks; n++) {
		Fork(cashierDo);
	}

	Fork(managerDo);
	
	return;
}