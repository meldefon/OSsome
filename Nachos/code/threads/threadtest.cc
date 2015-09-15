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

	//Thread* t = new Thread("Stupid thread");
	//t->Fork((VoidFunctionPtr)customer,0);

  	int size; //will be used to take in user input for the sizes of specific variables
	cout<<"How many Application Clerks would you like to have? ";
	cin >> size;

	appClerk.initialize("Application Clerk Line Lock", size);

	cout<<"How many Picture Clerks would you like to have? ";
	cin >> size;

	picClerk.initialize("Picture Clerk Line Lock", size);

	cout<<"How many Passport Clerks would you like to have? ";
	cin >> size;

	passPClerk.initialize("Passport Clerk Line Lock", size);

	cout<<"How many Cashiers would you like to have? ";
	cin >> size;

	cashier.initialize("Cashier Line Lock", size);

	cout<<"How many Customers would you like to have? ";
	cin >> size;

	//will hold booleans that indicate whether a customer has
	//completed application or pictures using the social security
	//number as an index
	customersWithCompletedApps = new bool[size];
	customersWithCompletedPics = new bool[size];
	passportClerkChecked = new bool[size];
	
	//initialize all the threads here
	return;
}