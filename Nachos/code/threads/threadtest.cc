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
	cout<<"How many Application Clerks would you like to have? ";
	cin >> size;

	appClerk.initialize("Application Clerk Line Lock","App", size);

	cout<<"How many Picture Clerks would you like to have? ";
	cin >> size;

	picClerk.initialize("Picture Clerk Line Lock","Pic", size);

	cout<<"How many Passport Clerks would you like to have? ";
	cin >> size;

	passPClerk.initialize("Passport Clerk Line Lock","Passport", size);

	cout<<"How many Cashiers would you like to have? ";
	cin >> size;

	cashier.initialize("Cashier Line Lock","Cashier", size);

	cout<<"How many Customers would you like to have? ";
	cin >> size;

	//will hold booleans that indicate whether a customer has
	//completed application or pictures using the social security
	//number as an index
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
	
	for(int i = 0; i < size; i++) {
		c = new Thread("Customer Thread");
		c->Fork((VoidFunctionPtr)customer,i);
	}

	return;
}


