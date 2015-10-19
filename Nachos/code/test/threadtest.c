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

void TestSuite() {

	int userChoice;
	int i;
	userChoice = 1;
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
		userChoice = Scanf();

		if(userChoice == 1) { /*shortest line*/
			
			/*initialize data for clerks, but not the clerk threads*/
			initialize(&appClerk, "Application Clerk Line Lock",0, 3);
			initialize(&picClerk, "Picture Clerk Line Lock",1, 3);
			
			/*initialze globals
			customersWithCompletedApps = new bool[10];
			customersWithCompletedPics = new bool[10];
			isSenator = new bool[10];
			*/
			numCustomersLeft = 10;
			senatorWorking = NULL;
			clerksCanWork = 1;
			newCustomerId = 0;
			newCustomerIdLock = CreateLock();
			senatorLock = CreateLock();
			senatorCV = CreateCondition();

			/*initialize customer threads*/
			for(i = 0; i < 10; i++) {
				customersWithCompletedApps[i] = 0;
				customersWithCompletedPics[i] = 0;

				if(i<10){
					isSenator[i]=0;
				}
				else{
					isSenator[i] = 1;
				}
				Fork(customer);
			}

			userChoice = 8;
		} else if(userChoice == 2) {
			/*This test sends 5 customers in plus 1 of each type of clerk. Demonstrates that each type of
			clerk's money is tracked correctly by the manager*/
			initialize(&cashier,"Cashier Line Lock",3, 1);
			initialize(&picClerk,"Picture Clerk Line Lock",1, 1);
			initialize(&passPClerk,"Passport Clerk Line Lock",2, 1);
			initialize(&appClerk,"Application Clerk Line Lock",0, 1);

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
			newCustomerId = 0;
			newCustomerIdLock = CreateLock();	

			Fork(applicationClerk);
			Fork(pictureClerk);
			Fork(passportClerk);
			Fork(cashierDo);

			/*make five customers*/
			for(i = 0; i < 5; i++) {
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

				Fork(customer);
			}
			
			Fork(managerDo);
			userChoice = 8;
		} else if(userChoice == 3) { /*cashier and customer passport test*/

			initialize(&cashier,"Cashier Line Lock",3, 1);

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
			newCustomerId = 0;
			newCustomerIdLock = CreateLock();

			/*make five customers*/
			for(i = 0; i < 5; i++) {
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

				Fork(customer);
			}

			/*make one cashier*/
			Fork(cashierDo);	
			userChoice = 8;	
		} else if(userChoice == 4) { /*clerks go on break when no one is in line*/

			initialize(&appClerk,"Application Clerk Line Lock",0, 1);
			initialize(&picClerk,"Picture Clerk Line Lock",1, 1);
			initialize(&passPClerk,"Passport Clerk Line Lock",2, 1);
			initialize(&cashier,"Cashier Line Lock",3, 1);

			/*isSenator = new bool[1];*/
			isSenator[0] = 0;
			numCustomersLeft = 1;
			senatorWorking = NULL;
			clerksCanWork = 1;
			senatorLock = CreateLock();
			senatorCV = CreateCondition();
			newCustomerId = 0;
			newCustomerIdLock = CreateLock();	

			for(i = 0; i < appClerk.numOfClerks; i++) {
				Fork(applicationClerk);
			}

			for(i = 0; i < picClerk.numOfClerks; i++) {
				Fork(pictureClerk);
			}

			for(i = 0; i < passPClerk.numOfClerks; i++) {
				Fork(passportClerk);
			}

			for(i = 0; i < cashier.numOfClerks; i++) {
				Fork(cashierDo);
			}

			userChoice = 8;
		} else if(userChoice == 5) { /*manager gets clerk off break*/
			
			initialize(&cashier,"Cashier Line Lock",3, 1);
			initialize(&picClerk,"Picture Clerk Line Lock",1, 1);
			initialize(&passPClerk,"Passport Clerk Line Lock",2, 1);
			initialize(&appClerk,"Application Clerk Line Lock",0, 1);

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
			newCustomerId = 0;
			newCustomerIdLock = CreateLock();
			
			/*make one cashier*/
			Fork(cashierDo);
			
			/*make five customers*/
			for(i = 0; i < 5; i++) {
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
				Fork(customer);
			}

			Fork(managerDo);
			userChoice = 8;

		} else if(userChoice == 6) {
			/*This test sends 4 customers in plus 1 senator*/
			int numCustsForTest; 
			initialize(&cashier,"Cashier Line Lock",3, 1);
			initialize(&picClerk,"Picture Clerk Line Lock",1, 1);
			initialize(&passPClerk,"Passport Clerk Line Lock",2, 1);
			initialize(&appClerk,"Application Clerk Line Lock",0, 1);

			numCustsForTest = 10;
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
			newCustomerId = 0;
			newCustomerIdLock = CreateLock();
			
			Fork(applicationClerk);
			Fork(pictureClerk);
			Fork(passportClerk);
			Fork(cashierDo);

			/*make five customers*/
			for(i = 0; i < numCustsForTest; i++) {
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
				
				Fork(customer);
			}

			Fork(managerDo);
			userChoice = 8;


		} else if(userChoice == 7) {
			/*This test sends 4 customers in plus 1 senator*/
			initialize(&cashier,"Cashier Line Lock",3, 1);
			initialize(&picClerk,"Picture Clerk Line Lock",1, 1);
			initialize(&passPClerk,"Passport Clerk Line Lock",2, 1);
			initialize(&appClerk,"Application Clerk Line Lock",0, 1);

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
			newCustomerId = 0;
			newCustomerIdLock = CreateLock();
			
			Fork(applicationClerk);
			Fork(pictureClerk);
			Fork(passportClerk);
			Fork(cashierDo);

			/*make five customers*/
			for(i = 0; i < 5; i++) {
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

				Fork(customer);
			}

			Fork(managerDo);
			userChoice = 8;
		}
	}
}


int main() {

  	int size; /*will be used to take in user input for the sizes of specific variables*/
  	int senatorSize;
	int i;
    int j;
    int k;
    int l;
    int m;
    int n;
	int numberOfSenators;
	int testSuite;

	Uprintf("For TestSuite, enter 1\nFor Simulation, enter 2", 46, 0, 0, 0, 0);
	testSuite = Scanf();

	if(testSuite == 1)
		TestSuite();
	else {

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
}
