#include "copyright.h"
#include "system.h"
#include <iostream>
using namespace std;
#include "synch.h"
#include "monitor.h"
#include "customer.cpp"
#include "clerks.cpp"
#include "globalVars.h"

void ThreadTest() {

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

	cout<<"Number of Senators = ";
	cin >> senatorSize;

	//will hold booleans that indicate whether a customer has
	//completed application or pictures using the social security
	//number as an index
	numCustomersLeft = size;
	customersWithCompletedApps = new bool[size];
	customersWithCompletedPics = new bool[size];
	passportClerkChecked = new bool[size];
	cashierChecked = new bool[size];
	gottenPassport = new bool[size];
	cashReceived = new int[size];

	//Initialize everything
	for(int i = 0;i<size;i++) {
		customersWithCompletedApps[i] = false;
		customersWithCompletedPics[i] = false;
		passportClerkChecked[i] = false;
		cashierChecked[i] = false;
		gottenPassport[i] = false;
		cashReceived[i] = 0;
	}


	//will hold currentCust SSN for checking
	appClerkCurrentCustomer = new int[appClerk.numOfClerks];
	pictureClerkCurrentCustomer = new int[picClerk.numOfClerks];
	passportClerkCurrentCustomer = new int[passPClerk.numOfClerks];
	cashierCurrentCustomer = new int[cashier.numOfClerks];

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


