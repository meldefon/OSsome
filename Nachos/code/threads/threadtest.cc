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
#include <string.h>

void sendReply(PacketHeader* outPktHdr,MailHeader* outMailHdr,stringstream& replyStream){

	int msgLength = replyStream.str().length()+1;
	outMailHdr->length = msgLength;
	char replyMsg[msgLength];
	strcpy(replyMsg,replyStream.str().c_str());
	bool success = postOffice->Send(*outPktHdr, *outMailHdr, replyMsg);
	if ( !success ) {
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}

}

void Server() {

	//Initialization
	vector<ServerLock*>* serverLocks = new vector<ServerLock*>;
	vector<ServerCV*>* serverCVs = new vector<ServerCV*>;
	vector<ServerMV*>* serverMVs = new vector<ServerMV*>;


	cout << "Running server\n";


	while (true) {

		//Vars for holding message details
		PacketHeader* outPktHdr = new PacketHeader(); 
		PacketHeader* inPktHdr = new PacketHeader();
		MailHeader* outMailHdr = new MailHeader();
		MailHeader* inMailHdr = new MailHeader();
		char buffer[MaxMailSize];

		// Wait for message from a client machine
		postOffice->Receive(0, inPktHdr, inMailHdr, buffer);
		DEBUG('S',"Got \"%s\" from %d, box %d\n", buffer, inPktHdr->from, inMailHdr->from);

		// Send acknowledgement to the other machine (using "reply to" mailbox
		// in the message that just arrived
		outPktHdr->to = inPktHdr->from;
		outMailHdr->to = inMailHdr->from;
		outMailHdr->from = 0;

		//Pull out message type
		int type;
		stringstream ss; //new one every time to be safe
		ss<<buffer;
		ss>>type;

		//Decide what to do based on message type
		string name;
		int lockNum, cvNum, mvSiz, mvNum, mvPos, mvVal;
		stringstream replyStream;
		switch (type){
			case SC_CreateLock: {
				DEBUG('S', "Message: Create lock\n");
				ss.get();
				getline(ss, name, '@'); //get name of lock
				DEBUG('T', "Creating lock %s for machine %d\n",name.c_str(),inPktHdr->from);


				//Check to see if that lock exists already
				int existingLockID = -1;
				for(int i = 0;i<serverLocks->size();i++){
					if(name.compare(serverLocks->at(i)->name)==0){
						existingLockID = i;
						break;
					}
				}

				//If doesn't, make a new lock
				if(existingLockID==-1) {
					//TODO Check to see if there's already a lock with this name
					//Create Lock, don't register ownerID or ownerMailbox
					ServerLock *newLock = new ServerLock;
					newLock->name = name;
					newLock->packetWaitQ = new queue<PacketHeader *>();
					newLock->mailWaitQ = new queue<MailHeader *>();
					newLock->state = Available;
					newLock->isToBeDeleted = false;
					newLock->ownerMachineID = -1;

					//Add to vector
					serverLocks->push_back(newLock);

					//Send reply - copy this template
					replyStream << serverLocks->size() - 1;
				}
				else{ //Lock does exist, so just give its ID
					replyStream << existingLockID;
				}

				sendReply(outPktHdr, outMailHdr, replyStream);
				break;
			}
			case SC_DestroyLock: {
				DEBUG('S', "Message: Destroy lock\n");
				ss >> lockNum; //get lock ID
				DEBUG('T', "Destroy lock %s for machine %d\n",name.c_str(),inPktHdr->from);


				//Validate user input: send -1 if bad
				if(lockNum < 0 || lockNum >= serverLocks->size()) {
					replyStream << -1;
				} else { 
					//Validate whether or not the lock exists
					if(serverLocks->at(lockNum) == NULL) {
						replyStream << -1;
					} else {
						serverLocks->at(lockNum)->isToBeDeleted = true;
						replyStream << -2;
					}
				}
				sendReply(outPktHdr, outMailHdr, replyStream);
				break;
			}
			case SC_CreateCondition: {
				DEBUG('S', "Message: Create condition\n");
				ss.get();
				getline(ss, name, '@'); //get name of CV


				int existingCVID = -1;

				//Once again, we have to check to see if there's already a CV made with this name
				for(int i = 0; i < serverCVs->size(); i++) {
					if (name.compare(serverCVs->at(i)->name) == 0) {
						existingCVID = i;
						break;
					}
				}


				//If CV doesn't exist, make a new one
				if(existingCVID==-1) {
					//Create Condition
					ServerCV *newCV = new ServerCV;
					newCV->name = name;
					newCV->packetWaitQ = new queue<PacketHeader *>();
					newCV->mailWaitQ = new queue<MailHeader *>();
					newCV->isToBeDeleted = false;
					newCV->lockID = -1;

					//Add to vector
					serverCVs->push_back(newCV);

					//Send reply - copy this template
					replyStream << serverCVs->size() - 1;
				}
				else{ //CV does exist, so give its ID
					replyStream<<existingCVID;
				}
				sendReply(outPktHdr, outMailHdr, replyStream);
				break;
			}
			case SC_DestroyCondition: {
				DEBUG('S', "Message: Destroy Condition\n");
				ss >> cvNum; //get lock ID

				//Validate user input: send -1 if bad
				if(cvNum < 0 || cvNum >= serverCVs->size()) {
					replyStream << -1;
				} else { 
					//Validate whether or not the CV exists
					if(serverCVs->at(cvNum) == NULL) {
						replyStream << -1;
					} else {
						serverCVs->at(cvNum)->isToBeDeleted = true;
						replyStream << -2;
					}
				}
				sendReply(outPktHdr, outMailHdr, replyStream);
				break;			
			}
			case SC_Acquire: {
				DEBUG('S', "Message: Acquire\n");
				ss >> lockNum; //get lock ID
				DEBUG('T', "Acquire lock %s for machine %d\n",serverLocks->at(lockNum)->name.c_str(),inPktHdr->from);


				bool ifReply = true;

				//Validate user input: send -1 if bad
				if(lockNum < 0 || lockNum >= serverLocks->size()) {
					replyStream << -1;
				} else { 
					//Check whether or not we can acquire it
					if(serverLocks->at(lockNum) == NULL) {
						replyStream << -1;
					} else if(serverLocks->at(lockNum)->ownerMachineID == outPktHdr->to && serverLocks->at(lockNum)->state == Busy) {
						//TODO add check int he else if above to make sure not just ownerMachineID, but some kind of
						//TODO thread-specific id matches
						replyStream << -1;
					} else if(serverLocks->at(lockNum)->state == Busy) {
						//Go on the wait queue
						ifReply = false;
						serverLocks->at(lockNum)->packetWaitQ->push(outPktHdr);
						serverLocks->at(lockNum)->mailWaitQ->push(outMailHdr);
					} else { 
						//Assign ownership of the lock and change state
						serverLocks->at(lockNum)->ownerMachineID = outPktHdr->to;
						serverLocks->at(lockNum)->state = Busy;
						replyStream << -2;
					}
				}

				//Only sends a reply if we were able to acquire OR there was an error
				if(ifReply) {
					sendReply(outPktHdr, outMailHdr, replyStream);
				}
				break;
			}
			case SC_Release: {
				DEBUG('S', "Message: Release\n");
				ss >> lockNum; //get lock ID
				DEBUG('T', "Release lock %s for machine %d\n",serverLocks->at(lockNum)->name.c_str(),inPktHdr->from);


				//Validate user input: send -1 if bad
				if(lockNum < 0 || lockNum >= serverLocks->size()) {
					replyStream << -1;
				} else { 
					//Check whether or not we can release it
					if(serverLocks->at(lockNum) == NULL) {
						replyStream << -1;					
					} else if(serverLocks->at(lockNum)->state == Available || serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to) {
						replyStream << -1;
					} else { 
						replyStream << -2;
						//Check if anyone is waiting so they must be woken up
						if(serverLocks->at(lockNum)->packetWaitQ->empty()) {
							serverLocks->at(lockNum)->state = Available;
							serverLocks->at(lockNum)->ownerMachineID = -1;
						} else {
							//Change ownership and send message to waiting client
							PacketHeader* tempOutPktHdr = serverLocks->at(lockNum)->packetWaitQ->front();
							serverLocks->at(lockNum)->packetWaitQ->pop();
							MailHeader* tempOutMailHdr = serverLocks->at(lockNum)->mailWaitQ->front();
							serverLocks->at(lockNum)->mailWaitQ->pop();
							serverLocks->at(lockNum)->ownerMachineID = tempOutPktHdr->to;
							sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
						}
					}
				}
				sendReply(outPktHdr, outMailHdr, replyStream);
				break;
			}
			case SC_Signal: {
				DEBUG('S', "Message: Signal\n");
				ss >> cvNum >> lockNum; //get lock and CV num

				//Validate user input: send -1 if bad
				if(lockNum < 0 || lockNum >= serverLocks->size() || cvNum < 0 || cvNum >= serverCVs->size()) {
					replyStream << -1;
				} else {
					//Do some more checks to ensure we can signal, like checking if the lock owner matches and the lock id matches the CV lock id
					if(serverLocks->at(lockNum) == NULL || serverCVs->at(cvNum) == NULL) {
						replyStream << -1;
					} else if(serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to || serverCVs->at(cvNum)->lockID != lockNum) {
						replyStream << -1;						
					} else {
						//If there is a waiting client, send reply so they can wake and go on to acquire
						if(serverCVs->at(cvNum)->packetWaitQ->empty()) {
							replyStream << -1;
						} else {
							//Send message to waiting client
							replyStream << -2;						
							PacketHeader* tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
							serverCVs->at(cvNum)->packetWaitQ->pop();
							MailHeader* tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
							serverCVs->at(cvNum)->mailWaitQ->pop();
							sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);

							if(serverCVs->at(cvNum)->packetWaitQ->empty()) {
								serverCVs->at(cvNum)->lockID = -1;
							}
						}
					}
				}
				sendReply(outPktHdr, outMailHdr, replyStream);
				break;
			}
			case SC_Wait: {
				DEBUG('S', "Message: Wait\n");
				ss >> cvNum >> lockNum; //get lock and CV num

				bool ifReply = true;

				//Validate user input: send -1 if bad
				if(lockNum < 0 || lockNum >= serverLocks->size() || cvNum < 0 || cvNum >= serverCVs->size()) {
					replyStream << -1;
				} else {
					//Do some more checks to ensure we can wait
					if(serverLocks->at(lockNum) == NULL || serverCVs->at(cvNum) == NULL) {
						replyStream << -1;
					} else if(serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to || (serverCVs->at(cvNum)->lockID != lockNum && serverCVs->at(cvNum)->lockID != -1)) {
						//Enters this condition block if the lock owner does not match machine ID
						//And if the CV lock does not match lock ID and the lock is assigned
						//Which means it doesnt have index value of -1
						replyStream << -1;
					} else {
						ifReply = false;
						//If CV is unused, assign new lock 
						if(serverCVs->at(cvNum)->lockID == -1) {
							serverCVs->at(cvNum)->lockID = lockNum;
						}
						serverCVs->at(cvNum)->packetWaitQ->push(outPktHdr);
						serverCVs->at(cvNum)->mailWaitQ->push(outMailHdr);
					}
				}
				
				if(ifReply) {
					sendReply(outPktHdr, outMailHdr, replyStream);
				}	
				break;			
			}
			case SC_Broadcast: {
				DEBUG('S', "Message: Broadcast\n");
				ss >> cvNum >> lockNum; //get lock and CV num

				//Validate user input: send -1 if bad
				if(lockNum < 0 || lockNum >= serverLocks->size() || cvNum < 0 || cvNum >= serverCVs->size()) {
					replyStream << -1;
				} else {
					//Do some more checks to ensure we can broadcast
					if(serverLocks->at(lockNum) == NULL || serverCVs->at(cvNum) == NULL) {
						replyStream << -1;
					} else if(serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to || (serverCVs->at(cvNum)->lockID != lockNum && serverCVs->at(cvNum)->lockID != -1)) {
						replyStream << -1;						
					} else {
						//If there is a waiting client, send reply so they can wake and go on to acquire
						if(serverCVs->at(cvNum)->packetWaitQ->empty()) {
							replyStream << -1;						
						} else {
							//do a simple loop and wake everybody up by message
							while(!serverCVs->at(cvNum)->packetWaitQ->empty()) {
								replyStream << -2;						
								PacketHeader* tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
								serverCVs->at(cvNum)->packetWaitQ->pop();
								MailHeader* tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
								serverCVs->at(cvNum)->mailWaitQ->pop();
								sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
							}
							serverCVs->at(cvNum)->lockID = -1; //since we've woken everyone up, no one is waiting on the lock anymore
						}
					}
				}
				sendReply(outPktHdr, outMailHdr, replyStream);
				break;
			}
			case SC_CreateMV: {
				DEBUG('S', "Message: CreateMV\n");
				ss.get();
				getline(ss, name, '@'); //get name of lock
				ss >> mvSiz;

				int existingMVID = -1;

				//Once again, we have to check to see if there's already a MV made with this name
				for(int i = 0; i < serverMVs->size(); i++) {
					if (name.compare(serverMVs->at(i)->name) == 0) {
						existingMVID = i;
						break;
					}
				}


				//If MV doesn't exist, create one
				if(existingMVID==-1) {

					//Create MV
					ServerMV *newMV = new ServerMV;
					newMV->name = name;
					newMV->vals = new int[mvSiz];
					newMV->length = mvSiz;
					newMV->isToBeDeleted = false;

					//Add to vector
					serverMVs->push_back(newMV);

					//Send reply - copy this template
					replyStream << serverMVs->size() - 1;
				}
				else {//MV Does exist, so return ID
					replyStream << existingMVID;
				}

				sendReply(outPktHdr, outMailHdr, replyStream);
				break;
			}
			case SC_DestroyMV: {
				DEBUG('S', "Message: DestroyMV\n");
				ss >> mvNum;

				//Validate user input: send -1 if bad
				if(mvNum < 0 || mvNum >= serverMVs->size()) {
					replyStream << -1;
				} else {
					//Do one more check before destroying
					if(serverMVs->at(mvNum) == NULL) {
						replyStream << -1;						
					} else {
						serverMVs->at(mvNum)->isToBeDeleted = true;
						replyStream << -2;						
					}
				}
				sendReply(outPktHdr, outMailHdr, replyStream);
				break;
			}
			case SC_SetMV: {
				DEBUG('S', "Message: SetMV\n");
				ss >> mvNum >> mvPos >> mvVal;
				
				//Validate user input: send -1 if bad
				if(mvNum < 0 || mvNum >= serverMVs->size() || mvPos < 0) {
					replyStream << -1;
				} else {
					//Do some more checks before setting value
					if(serverMVs->at(mvNum) == NULL) {
						replyStream << -1;						
					} else if(mvPos >= serverMVs->at(mvNum)->length) {
						replyStream << -1;						
					} else {
						serverMVs->at(mvNum)->vals[mvPos] = mvVal;
						replyStream << -2;						
					}
				}
				sendReply(outPktHdr, outMailHdr, replyStream);
				break;
			}
			case SC_GetMV: {
				DEBUG('S', "Message: GetMV\n");
				ss >> mvNum >> mvPos;
				
				//Validate user input: send -1 if bad
				if(mvNum < 0 || mvNum >= serverMVs->size() || mvPos < 0) {
					replyStream << -1;
				} else {
					//Do some more checks before setting value
					if(serverMVs->at(mvNum) == NULL) {
						replyStream << -1;						
					} else if(mvPos >= serverMVs->at(mvNum)->length) {
						replyStream << -1;						
					} else {
						replyStream << serverMVs->at(mvNum)->vals[mvPos];
					}
				}
				sendReply(outPktHdr, outMailHdr, replyStream);
				break;
			}
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
