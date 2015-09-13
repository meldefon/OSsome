#include "copyright.h"
#include "system.h"
#include <iostream>
using namespace std;
#include "synch.h"
#include "monitor.h"
//#include "globalVars.h"


void ThreadTest() {

	Monitor appClerk, picClerk, passPClerk, cashier;

//global shared data between the clerks that are used for filing purposes
	bool *customersWithCompletedApps;
	bool *customersWithCompletedPics;
	bool *passportClerkChecked;
	bool *gottenPassport;
	int *cashReceived;


  	int size; //will be used to take in user input for the sizes of specific variables
	cout<<"How many Application Clerks would you like to have? ";
	cin >> size;

	appClerk.lineLock = new Lock("AppClerk Line Lock");
	appClerk.lineCV = new Condition[size]();
	appClerk.clerkLock = new Lock[size]();
	appClerk.clerkCV = new Condition[size]();
	appClerk.lineCount = new int[size]();
	appClerk.bribeLineCount = new int[size]();
	appClerk.clerkState = new int[size]();

	cout<<"How many Picture Clerks would you like to have? ";
	cin >> size;

	picClerk.lineLock = new Lock("Picture Clerk Line Lock");
	picClerk.lineCV = new Condition[size]();
	picClerk.clerkLock = new Lock[size]();
	picClerk.clerkCV = new Condition[size]();
	picClerk.lineCount = new int[size]();
	picClerk.bribeLineCount = new int[size]();
	picClerk.clerkState = new int[size]();

	cout<<"How many Passport Clerks would you like to have? ";
	cin >> size;

	passPClerk.lineLock = new Lock("Passport Clerk Line Lock");
	passPClerk.lineCV = new Condition[size]();
	passPClerk.clerkLock = new Lock[size]();
	passPClerk.clerkCV = new Condition[size]();
	passPClerk.lineCount = new int[size]();
	passPClerk.bribeLineCount = new int[size]();
	passPClerk.clerkState = new int[size]();

	cout<<"How many Cashiers would you like to have? ";
	cin >> size;

	cashier.lineLock = new Lock("Cashier Line Lock");
	cashier.lineCV = new Condition[size]();
	cashier.clerkLock = new Lock[size]();
	cashier.clerkCV = new Condition[size]();
	cashier.lineCount = new int[size]();
	cashier.bribeLineCount = new int[size]();
	cashier.clerkState = new int[size]();

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