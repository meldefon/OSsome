#include "copyright.h"
#include "system.h"
#include <iostream>
#include <sstream>
#include <test_code.cc>
using namespace std;
#include "synch.h"
#include "monitor.h"
#include "customer.cpp"
#include "clerks.cpp"
#include "globalVars.h"
#include "serverStructs.h"
#include <vector>
#include "syscall.h"

void Server() {

	//Initialization
	vector<ServerLock>* serverLocks = new vector<ServerLock>;
	vector<ServerCV>* serverCVs = new vector<ServerCV>;
	vector<ServerMV>* serverMVs = new vector<ServerMV>;


	cout << "Running server\n";

	//Vars for holding message details
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char buffer[MaxMailSize];

	while (true) {
		// Wait for message from a client machine
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
		DEBUG('S',"Got \"%s\" from %d, box %d\n", buffer, inPktHdr.from, inMailHdr.from);

		//Pull out message type
		int type;
		stringstream ss; //new one every time to be safe
		ss<<buffer;
		ss>>type;

		//Decide what to do based on message type
		string name;
		int lockNum, cvNum;
		switch (type){
			case SC_CreateLock:
				DEBUG('S',"Message: Create lock\n");
				ss.get();
				getline(ss,name,'@'); //get name of lock

				break;
			case SC_DestroyLock:
				DEBUG('S',"Message: Destroy lock\n");
				ss>>lockNum; //get lock ID
				break;
			case SC_CreateCondition:
				DEBUG('S',"Message: Create condition\n");
				ss.get();
				getline(ss,name,'@'); //get name of CV

				break;
			case SC_DestroyCondition:
				DEBUG('S',"Message: Destroy Condition\n");
				ss>>cvNum; //get lock ID

				break;
			case SC_Acquire:
				DEBUG('S',"Message: Acquire\n");
				ss>>lockNum; //get lock ID

				break;
			case SC_Release:
				DEBUG('S',"Message: Release\n");
				ss>>lockNum; //get lock ID

				break;
			case SC_Signal:
				DEBUG('S',"Message: Signal\n");
				ss>>cvNum>>lockNum; //get lock and CV num

				break;
			case SC_Wait:
				DEBUG('S',"Message: Wait\n");
				ss>>cvNum>>lockNum; //get lock and CV num

				break;
			case SC_Broadcast:
				DEBUG('S',"Message: Broadcast\n");
				ss>>cvNum>>lockNum; //get lock and CV num

				break;

			default:
				cout<<"Unkonwn message type. Ignoring.\n";
				continue;
				break;
		}


	}



}

void ThreadTest() {
	//STestSuite();

  	int size; //will be used to take in user input for the sizes of specific variables
  	int senatorSize;

	cout<<"Number of ApplicationClerks = ";
	cin >> size;

	appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", size);

	cout<<"Number of PictureClerks = ";
	cin >> size;

	picClerk.initialize("Picture Clerk Line Lock","PictureClerk", size);

	cout<<"Number of PassportClerks = ";
	cin >> size;

	passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", size);

	cout<<"Number of Cashiers = ";
	cin >> size;

	cashier.initialize("Cashier Line Lock","Cashier", size);

	cout<<"Number of Customers = ";
	cin >> size;


	int numberOfSenators;
	cout<<"Number of Senators = ";
	cin >> numberOfSenators;

	size += numberOfSenators;


	//will hold booleans that indicate whether a customer has
	//completed application or pictures using the social security
	//number as an index
	numCustomersLeft = size;
	customersWithCompletedApps = new bool[size];
	//customersWithCompletedApps = (int*) malloc(size * sizeof(int));
	customersWithCompletedPics = new bool[size];
	//customersWithCompletedPics = (int*) malloc(size * sizeof(int));
	passportClerkChecked = new bool[size];
	//passportClerkChecked = (int*) malloc(size * sizeof(int));
	cashierChecked = new bool[size];
	//cashierChecked = (int*) malloc(size * sizeof(int));
	gottenPassport = new bool[size];
	//gottenPassport = (int*) malloc(size * sizeof(int));
	cashReceived = new int[size];
	//cashReceived = (int*) malloc(size * sizeof(int));
	isSenator = new bool[size];
	//isSenator = (int*) malloc(size * sizeof(int));
	bribesEnabled = true;
	//bribesEnabled = 1;

	//Initialize everything
	//int sizeOfInt = sizeof(int);
	for(int i = 0;i<size;i++) {
		customersWithCompletedApps[i] = false;
		//*(customersWithCompletedApps + (i * sizeOfInt)) = 0;
		customersWithCompletedPics[i] = false;
		//*(customersWithCompletedPics + (i * sizeOfInt)) = 0;
		passportClerkChecked[i] = false;
		//*(passportClerkChecked + (i * sizeOfInt)) = 0;
		cashierChecked[i] = false;
		//*(cashierChecked + (i * sizeOfInt)) = 0;
		gottenPassport[i] = false;
		//*(gottenPassport + (i * sizeOfInt)) = 0;		
		cashReceived[i] = 0;
		//*(cashReceived + (i * sizeOfInt)) = 0;		
		if(i<size-numberOfSenators){
			isSenator[i]=false;
			//*(isSenator + (i * sizeOfInt)) = 0;		
		}
		else{
			isSenator[i] = true;
			//*(isSenator + (i * sizeOfInt)) = 1;		
		}
	}
	senatorWorking = NULL;
	clerksCanWork = true;
	//clerksCanWork = 1;

	//Instantiate senator lock/CV
	senatorLock = new Lock("Senator lock");
	//senatorLock = CreateLock();
	senatorCV = new Condition("Senator CV");
	//senatorCV = CreateCondition();

	//will hold currentCust SSN for checking
	appClerkCurrentCustomer = new int[appClerk.numOfClerks];
	//appClerkCurrentCustomer = (int*) malloc(appClerk.numOfClerks * sizeOfInt);
	pictureClerkCurrentCustomer = new int[picClerk.numOfClerks];
	//pictureClerkCurrentCustomer = (int*) malloc(picClerk.numOfClerks * sizeOfInt);	
	passportClerkCurrentCustomer = new int[passPClerk.numOfClerks];
	//passportClerkCurrentCustomer = (int*) malloc(passPClerk.numOfClerks * sizeOfInt);	
	cashierCurrentCustomer = new int[cashier.numOfClerks];
	//cashierCurrentCustomer = (int*) malloc(cashier.numOfClerks * sizeOfInt);	

	//initialize all the threads here 
	Thread *c;

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

	return;
}

void TestSuite() {
/*
	Thread *c;
	int userChoice;
	cout<<"Test Suite\n\n";

	while(userChoice != 8) {

		cout<<"1. Customers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time\n.";
		cout<<"2. Managers only read one from one Clerk's total money received, at a time.\n.";
		cout<<"3. Customers do not leave until they are given their passport by the Cashier. The Cashier does not start on another customer until they know that the last Customer has left their area\n.";
 		cout<<"4. Clerks go on break when they have no one waiting in their line\n.";
 		cout<<"5. Managers get Clerks off their break when lines get too long\n.";
 		cout<<"6. Total sales never suffers from a race condition\n.";
 		cout<<"7. The behavior of Customers is proper when Senators arrive. This is before, during, and after\n.";
		cout<<"8. Quit.\n";
		cout<<"Pick a test by entering in the test number: ";
		cin >> userChoice;

		if(userChoice == 1) { //shortest line
			
			//initialize data for clerks, but not the clerk threads
			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 3);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 3);
			
			//initialze globals
			customersWithCompletedApps = new bool[10];
			customersWithCompletedPics = new bool[10];
			isSenator = new bool[10];
			numCustomersLeft = 10;
			senatorWorking = NULL;
			clerksCanWork = true;

			//initialize customer threads
			for(int i = 0; i < 10; i++) {
				customersWithCompletedApps[i] = false;
				customersWithCompletedPics[i] = false;

				if(i<10){
					isSenator[i]=false;
				}
				else{
					isSenator[i] = true;
				}

				c = new Thread("Customer Thread");
				c->Fork((VoidFunctionPtr)customer,i);
			}

			senatorLock = new Lock("Senator lock");
			senatorCV = new Condition("Senator CV");
			userChoice = 8;
		} else if(userChoice == 2) {
			//This test sends 5 customers in plus 1 of each type of clerk. Demonstrates that each type of
			//clerk's money is tracked correctly by the manager
			cashier.initialize("Cashier Line Lock","Cashier", 1);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 1);
			passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", 1);
			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 1);

			//initialze globals
			customersWithCompletedApps = new bool[5];
			customersWithCompletedPics = new bool[5];
			passportClerkChecked = new bool[5];
			cashierChecked = new bool[5];
			gottenPassport = new bool[5];
			cashReceived = new int[5];
			cashierCurrentCustomer = new int[1];
			numCustomersLeft = 5;
			isSenator = new bool[5];
			numCustomersLeft = 5;
			senatorWorking = NULL;
			clerksCanWork = true;
			senatorLock = new Lock("Senator lock");
			senatorCV = new Condition("Senator CV");

			//make one appClerk
			c = new Thread("AppClerk Thread");
			c->Fork((VoidFunctionPtr)applicationClerk, 0);

			//make one appClerk
			c = new Thread("PicClerk Thread");
			c->Fork((VoidFunctionPtr)pictureClerk, 0);

			//make one passportClerk
			c = new Thread("passportClerk Thread");
			c->Fork((VoidFunctionPtr)passportClerk, 0);

			//make one cashier
			c = new Thread("Cashier Thread");
			c->Fork((VoidFunctionPtr)cashierDo, 0);

			//make five customers
			for(int i = 0; i < 5; i++) {
				customersWithCompletedApps[i] = true;
				customersWithCompletedPics[i] = true;
				passportClerkChecked[i] = true;
				cashierChecked[i] = false;
				gottenPassport[i] = false;
				cashReceived[i] = 0;

				if(i<5){
					isSenator[i]=false;
				}
				else{
					isSenator[i] = true;
				}

				c = new Thread("Customer Thread");
				c->Fork((VoidFunctionPtr)customer,i);
			}

			c = new Thread("Manager Thread");
			c->Fork((VoidFunctionPtr)managerDo, 0);



			userChoice = 8;
		} else if(userChoice == 3) { //cashier and customer passport test

			cashier.initialize("Cashier Line Lock","Cashier", 1);

			//initialze globals
			customersWithCompletedApps = new bool[5];
			customersWithCompletedPics = new bool[5];
			passportClerkChecked = new bool[5];
			cashierChecked = new bool[5];
			gottenPassport = new bool[5];
			cashReceived = new int[5];
			cashierCurrentCustomer = new int[1];
			isSenator = new bool[5];
			numCustomersLeft = 5;
			senatorWorking = NULL;
			clerksCanWork = true;
			senatorLock = new Lock("Senator lock");
			senatorCV = new Condition("Senator CV");

			//make five customers
			for(int i = 0; i < 5; i++) {
				customersWithCompletedApps[i] = true;
				customersWithCompletedPics[i] = true;
				passportClerkChecked[i] = true;
				cashierChecked[i] = false;
				gottenPassport[i] = false;
				cashReceived[i] = 0;

				if(i<5){
					isSenator[i]=false;
				}
				else{
					isSenator[i] = true;
				}

				c = new Thread("Customer Thread");
				c->Fork((VoidFunctionPtr)customer,i);
			}

			//make one cashier
			c = new Thread("Cashier Thread");
			c->Fork((VoidFunctionPtr)cashierDo,0);	
			userChoice = 8;	
		} else if(userChoice == 4) { //clerks go on break when no one is in line

			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 1);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 1);
			passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", 1);
			cashier.initialize("Cashier Line Lock","Cashier", 1);

			isSenator = new bool[1];
			isSenator[0] = false;
			numCustomersLeft = 1;
			senatorWorking = NULL;
			clerksCanWork = true;
			senatorLock = new Lock("Senator lock");
			senatorCV = new Condition("Senator CV");

			//-1 indicates a test case
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
		} else if(userChoice == 5) { //manager gets clerk off break
			
			cashier.initialize("Cashier Line Lock","Cashier", 1);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 1);
			passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", 1);
			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 1);

			//initialze globals
			customersWithCompletedApps = new bool[5];
			customersWithCompletedPics = new bool[5];
			passportClerkChecked = new bool[5];
			cashierChecked = new bool[5];
			gottenPassport = new bool[5];
			cashReceived = new int[5];
			cashierCurrentCustomer = new int[1];
			numCustomersLeft = 5;
			isSenator = new bool[5];
			numCustomersLeft = 5;
			senatorWorking = NULL;
			clerksCanWork = true;
			senatorLock = new Lock("Senator lock");
			senatorCV = new Condition("Senator CV");

			//make one cashier
			c = new Thread("Cashier Thread");
			c->Fork((VoidFunctionPtr)cashierDo, 0);
			
			//make five customers
			for(int i = 0; i < 5; i++) {
				customersWithCompletedApps[i] = true;
				customersWithCompletedPics[i] = true;
				passportClerkChecked[i] = true;
				cashierChecked[i] = false;
				gottenPassport[i] = false;
				cashReceived[i] = 0;
				
				if(i<5){
					isSenator[i]=false;
				}
				else{
					isSenator[i] = true;
				}
				
				c = new Thread("Customer Thread");
				c->Fork((VoidFunctionPtr)customer,i);
			}

			c = new Thread("Manager Thread");
			c->Fork((VoidFunctionPtr)managerDo, 0);
			userChoice = 8;

		} else if(userChoice == 6) {
			//This test sends 4 customers in plus 1 senator
			cashier.initialize("Cashier Line Lock","Cashier", 1);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 1);
			passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", 1);
			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 1);

			bribesEnabled = false;
			//initialze globals
			int numCustsForTest = 10;
			customersWithCompletedApps = new bool[numCustsForTest];
			customersWithCompletedPics = new bool[numCustsForTest];
			passportClerkChecked = new bool[numCustsForTest];
			cashierChecked = new bool[numCustsForTest];
			gottenPassport = new bool[numCustsForTest];
			cashReceived = new int[numCustsForTest];
			cashierCurrentCustomer = new int[1];
			numCustomersLeft = numCustsForTest;
			isSenator = new bool[numCustsForTest];
			numCustomersLeft = numCustsForTest;
			senatorWorking = NULL;
			clerksCanWork = true;
			senatorLock = new Lock("Senator lock");
			senatorCV = new Condition("Senator CV");

			//make one appClerk
			c = new Thread("AppClerk Thread");
			c->Fork((VoidFunctionPtr)applicationClerk, 0);

			//make one appClerk
			c = new Thread("PicClerk Thread");
			c->Fork((VoidFunctionPtr)pictureClerk, 0);

			//make one passportClerk
			c = new Thread("passportClerk Thread");
			c->Fork((VoidFunctionPtr)passportClerk, 0);

			//make one cashier
			c = new Thread("Cashier Thread");
			c->Fork((VoidFunctionPtr)cashierDo, 0);

			//make five customers
			for(int i = 0; i < numCustsForTest; i++) {
				customersWithCompletedApps[i] = true;
				customersWithCompletedPics[i] = true;
				passportClerkChecked[i] = true;
				cashierChecked[i] = false;
				gottenPassport[i] = false;
				cashReceived[i] = 0;

				if(i<numCustsForTest){
					isSenator[i]=false;
				}
				else{
					isSenator[i] = true;
				}

				c = new Thread("Customer Thread");
				c->Fork((VoidFunctionPtr)customer,i);
			}

			c = new Thread("Manager Thread");
			c->Fork((VoidFunctionPtr)managerDo, 0);
			userChoice = 8;


		} else if(userChoice == 7) {
			//This test sends 4 customers in plus 1 senator
			cashier.initialize("Cashier Line Lock","Cashier", 1);
			picClerk.initialize("Picture Clerk Line Lock","PictureClerk", 1);
			passPClerk.initialize("Passport Clerk Line Lock","PassportClerk", 1);
			appClerk.initialize("Application Clerk Line Lock","ApplicationClerk", 1);

			//initialze globals
			customersWithCompletedApps = new bool[5];
			customersWithCompletedPics = new bool[5];
			passportClerkChecked = new bool[5];
			cashierChecked = new bool[5];
			gottenPassport = new bool[5];
			cashReceived = new int[5];
			cashierCurrentCustomer = new int[1];
			numCustomersLeft = 5;
			isSenator = new bool[5];
			numCustomersLeft = 5;
			senatorWorking = NULL;
			clerksCanWork = true;
			senatorLock = new Lock("Senator lock");
			senatorCV = new Condition("Senator CV");

			//make one appClerk
			c = new Thread("AppClerk Thread");
			c->Fork((VoidFunctionPtr)applicationClerk, 0);

			//make one appClerk
			c = new Thread("PicClerk Thread");
			c->Fork((VoidFunctionPtr)pictureClerk, 0);

			//make one passportClerk
			c = new Thread("passportClerk Thread");
			c->Fork((VoidFunctionPtr)passportClerk, 0);

			//make one cashier
			c = new Thread("Cashier Thread");
			c->Fork((VoidFunctionPtr)cashierDo, 0);

			//make five customers
			for(int i = 0; i < 5; i++) {
				customersWithCompletedApps[i] = true;
				customersWithCompletedPics[i] = true;
				passportClerkChecked[i] = true;
				cashierChecked[i] = false;
				gottenPassport[i] = false;
				cashReceived[i] = 0;

				if(i<4){
					isSenator[i]=false;
				}
				else{
					isSenator[i] = true;
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
*/
}
