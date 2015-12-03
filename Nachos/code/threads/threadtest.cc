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
	/*if(outPktHdr->to<NUM_SERVERS){
		outMailHdr->to = 0;
	}*/
	bool success = postOffice->Send(*outPktHdr, *outMailHdr, replyMsg);
	if ( !success ) {
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}

}
void sendReplyToServer(PacketHeader* outPktHdr, MailHeader* outMailHdr, int requestType, int requestID, int machineID, int mailbox, int reply) {
	stringstream replyStream; 
	replyStream << requestType << " " << requestID << " " << machineID <<  " " << mailbox << " " << reply;
	//outMailHdr->from = 0; //We need to send the correct return mailbox
	sendReply(outPktHdr, outMailHdr, replyStream);
}

void sendReplyToClient(int machineID, int mailbox, int reply) {
	
	PacketHeader* outPktHdr = new PacketHeader(); 
	MailHeader* outMailHdr = new MailHeader();
			
	outPktHdr->to = machineID; //client machineID
	outMailHdr->to = mailbox; //client mailbox
	outMailHdr->from = myMachineID; // server machineID goes here

	stringstream replyStream;
	replyStream << reply;
	sendReply(outPktHdr, outMailHdr, replyStream);
}

void NewServerRequest(vector<ServerRequest*>* serverRQs, string name, int requestType, int machineID, int mailbox, int arg1, int arg2, int arg3) {
	
	//Create new serverRequest object and add it to server's data
	ServerRequest *sr = new ServerRequest;
	serverRQs->push_back(sr);

	//initialize values
	sr->requestID = serverRQs->size() - 1;	
	sr->machineID = machineID;
	sr->mailbox = mailbox;
	sr->requestType = requestType;
	sr->arg1 = arg1;
	sr->arg2 = arg2;
	sr->arg3 = arg3;
	sr->noCount = 0;
	sr->yes = false;
	sr->name = name;
	sr->lockCount = 0;
	sr->cvCount = 0;
	sr->lockFound = false;
	sr->cvFound = false;

	//send serverRequest to all other servers
	for(int i = 0; i < NUM_SERVERS; i++) {
		//if not my machineID, send request to this machineID
		if(i != myMachineID) {
			//create packets to send to server
			PacketHeader* outPktHdr = new PacketHeader(); 
			MailHeader* outMailHdr = new MailHeader();
			
			outPktHdr->to = i; //other server machineID goes here
			outMailHdr->to = 0; //default mailbox is zero
			//outMailHdr->to = i; //send to other server's mailbox
			outMailHdr->from = myMachineID; //our server machineID goes here
			//outMailHdr->from = 0; //reply mail needs to come here

			//create stringstream object to store data, use an if statement to send different data depending on what the syscall is
			stringstream ss;
			
			if(requestType == SC_Server_CreateCondition || requestType == SC_Server_CreateLock || requestType == SC_Server_CreateMV) {
				ss << sr->requestType << " " << sr->requestID << " " << sr->machineID << " " << sr->mailbox << " " << sr->name << "@";
			} else if(requestType == SC_Server_Acquire || requestType == SC_Server_Release || requestType == SC_Server_DestroyLock || requestType == SC_Server_DestroyCondition || requestType == SC_Server_DestroyMV) {
				ss << sr->requestType << " " << sr->requestID << " " << sr->machineID << " " << sr->mailbox << " " << sr->arg1; 
			} else if(requestType == SC_Server_GetMV || requestType == SC_Server_Wait1 || requestType == SC_Server_Wait2 || requestType == SC_Server_Wait3 || requestType == SC_Server_Signal1 || requestType == SC_Server_Signal2 || requestType == SC_Server_Signal3 || requestType == SC_Server_Broadcast1 || requestType == SC_Server_Broadcast2 || requestType == SC_Server_Broadcast3) {
				ss << sr->requestType << " " << sr->requestID << " " << sr->machineID << " " << sr->mailbox << " " << sr->arg1 << " " << sr->arg2;
			} else {
				ss << sr->requestType << " " << sr->requestID << " " << sr->machineID << " " << sr->mailbox << " " << sr->arg1 << " " << sr->arg2 << " " << sr->arg3;
			}

			sendReply(outPktHdr, outMailHdr, ss);
		}
	}
}

bool checkIfEnter(vector<ServerRequest*>* serverRQs, vector<ServerLock*>* serverLocks, vector<ServerCV*>* serverCVs, int cvNum, int lockNum, int machineID, int mailbox, int requestType, int offset) {
	//do some quick checks before we send server request, some checks are repeated later down the line but it's alright
	//it's alright because we are not Bill Gates nor Steve Jobs
	bool ifEnter = true;

	//check which cases we may be dealing with in regard to finding locks or CVs on diff servers
	if(lockNum / 100 != myMachineID || cvNum / 100 != myMachineID) {
		ifEnter = false;
		//check what is missing
		if(lockNum / 100 != myMachineID && cvNum / 100 != myMachineID) {
			NewServerRequest(serverRQs, "", requestType + offset + 1, machineID, mailbox, cvNum, lockNum, 0);
		} else if(lockNum / 100 != myMachineID) {
			//Lock doesn't exist but CV does, so let's check to make sure CV info is valid
			int cvID = cvNum;
			cvNum = cvNum % 100;
			
			if (cvNum < 0 || cvNum >= serverCVs->size()) {
				sendReplyToClient(machineID, mailbox, -1);
			} else if(serverCVs->at(cvNum) == NULL) {
				sendReplyToClient(machineID, mailbox, -1);
			} else if((requestType == SC_Server_Signal1 && serverCVs->at(cvNum)->lockID != lockNum) || (requestType == SC_Server_Wait1 && serverCVs->at(cvNum)->lockID != lockNum && serverCVs->at(cvNum)->lockID != -1)) {
				sendReplyToClient(machineID, mailbox, -1);
			} else {
				NewServerRequest(serverRQs, "", requestType + offset, machineID, mailbox, cvID, lockNum, 0);
			}
		} else if(cvNum / 100 != myMachineID) {
			//CV doesn't exist but lock does, so let's check to make sure lock info is valid
			int lockID = lockNum;
			lockNum = lockNum % 100;

			if (lockNum < 0 || lockNum >= serverLocks->size()) {
				sendReplyToClient(machineID, mailbox, -1);
			} else if(serverLocks->at(lockNum) == NULL) {
				sendReplyToClient(machineID, mailbox, -1);
			} else if(serverLocks->at(lockNum)->ownerMachineID != machineID || serverLocks->at(lockNum)->ownerMailboxNum != mailbox) {
				sendReplyToClient(machineID, mailbox, -1);
			} else {
				NewServerRequest(serverRQs, "", requestType, machineID, mailbox, cvNum, lockID, 0);
			}
		}
	}

	return ifEnter;
}

void Server() {

	if(NUM_SERVERS==1) {

		//Initialization
		vector<ServerLock *> *serverLocks = new vector<ServerLock *>;
		vector<ServerCV *> *serverCVs = new vector<ServerCV *>;
		vector<ServerMV *> *serverMVs = new vector<ServerMV *>;


		cout << "Running server\n";
		cout << "Set to handle " << NUM_SERVERS << " servers.\n";


		while (true) {

			//Vars for holding message details
			PacketHeader *outPktHdr = new PacketHeader();
			PacketHeader *inPktHdr = new PacketHeader();
			MailHeader *outMailHdr = new MailHeader();
			MailHeader *inMailHdr = new MailHeader();
			char buffer[MaxMailSize];

			// Wait for message from a client machine
			postOffice->Receive(0, inPktHdr, inMailHdr, buffer);
			DEBUG('S', "Got \"%s\" from %d, box %d\n", buffer, inPktHdr->from, inMailHdr->from);

			// Send acknowledgement to the other machine (using "reply to" mailbox
			// in the message that just arrived
			outPktHdr->to = inPktHdr->from;
			outMailHdr->to = inMailHdr->from;
			outMailHdr->from = 0;

			//Pull out message type
			int type;
			stringstream ss; //new one every time to be safe
			ss << buffer;
			ss >> type;

			//Decide what to do based on message type
			string name;
			int lockNum, cvNum, mvSiz, mvNum, mvPos, mvVal;
			stringstream replyStream;
			//First if statement does server vs. client request code
			if (type % 100 == type) {

				switch (type) {
					case SC_CreateLock: {
						DEBUG('S', "Message: Create lock\n");
						ss.get();
						getline(ss, name, '@'); //get name of lock
						DEBUG('T', "Creating lock %s for machine %d, mailbox %d\n", name.c_str(), inPktHdr->from,
							  inMailHdr->from);


						//Check to see if that lock exists already
						int existingLockID = -1;
						for (int i = 0; i < serverLocks->size(); i++) {
							if (name.compare(serverLocks->at(i)->name) == 0) {
								existingLockID = i;
								break;
							}
						}

						//If doesn't, make a new lock
						if (existingLockID == -1) {
							//TODO Check to see if there's already a lock with this name
							//Create Lock, don't register ownerID or ownerMailbox
							ServerLock *newLock = new ServerLock;
							newLock->name = name;
							newLock->packetWaitQ = new queue<PacketHeader *>();
							newLock->mailWaitQ = new queue<MailHeader *>();
							newLock->state = Available;
							newLock->isToBeDeleted = false;
							newLock->ownerMachineID = -1;
							newLock->ownerMailboxNum = -1;

							//Add to vector
							serverLocks->push_back(newLock);

							//Send reply - copy this template
							replyStream << serverLocks->size() - 1;
						}
						else { //Lock does exist, so just give its ID
							replyStream << existingLockID;
						}

						sendReply(outPktHdr, outMailHdr, replyStream);
						break;
					}
					case SC_DestroyLock: {
						DEBUG('S', "Message: Destroy lock\n");
						ss >> lockNum; //get lock ID
						DEBUG('T', "Set destroy lock %s for machine %d, mailbox %d\n",
							  serverLocks->at(lockNum)->name.c_str(), inPktHdr->from, inMailHdr->from);


						//Validate user input: send -1 if bad
						if (lockNum < 0 || lockNum >= serverLocks->size()) {
							replyStream << -1;
						} else {
							//Validate whether or not the lock exists
							if (serverLocks->at(lockNum) == NULL) {
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
						DEBUG('T', "Creating CV %s for machine %d, mailbox %d\n", name.c_str(), inPktHdr->from,
							  inMailHdr->from);


						int existingCVID = -1;

						//Once again, we have to check to see if there's already a CV made with this name
						for (int i = 0; i < serverCVs->size(); i++) {
							if (name.compare(serverCVs->at(i)->name) == 0) {
								existingCVID = i;
								break;
							}
						}


						//If CV doesn't exist, make a new one
						if (existingCVID == -1) {
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
						else { //CV does exist, so give its ID
							replyStream << existingCVID;
						}
						sendReply(outPktHdr, outMailHdr, replyStream);
						break;
					}
					case SC_DestroyCondition: {
						DEBUG('S', "Message: Destroy Condition\n");
						ss >> cvNum; //get lock ID
						DEBUG('T', "Set destroy CV %s for machine %d, mailbox %d\n", serverCVs->at(cvNum)->name.c_str(),
							  inPktHdr->from, inMailHdr->from);

						//Validate user input: send -1 if bad
						if (cvNum < 0 || cvNum >= serverCVs->size()) {
							replyStream << -1;
						} else {
							//Validate whether or not the CV exists
							if (serverCVs->at(cvNum) == NULL) {
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
						DEBUG('T', "Acquire lock %s for machine %d, mailbox %d\n",
							  serverLocks->at(lockNum)->name.c_str(),
							  inPktHdr->from, inMailHdr->from);


						bool ifReply = true;

						//Validate user input: send -1 if bad
						if (lockNum < 0 || lockNum >= serverLocks->size()) {
							replyStream << -1;
						} else {
							//Check whether or not we can acquire it
							if (serverLocks->at(lockNum) == NULL) {
								replyStream << -1;
							} else if (serverLocks->at(lockNum)->ownerMachineID == outPktHdr->to &&
									   serverLocks->at(lockNum)->ownerMailboxNum == outMailHdr->to &&
									   serverLocks->at(lockNum)->state == Busy) {
								//TODO add check int he else if above to make sure not just ownerMachineID, but some kind of
								//TODO thread-specific id matches
								replyStream << -1;
							} else if (serverLocks->at(lockNum)->state == Busy) {
								//Go on the wait queue
								ifReply = false;
								serverLocks->at(lockNum)->packetWaitQ->push(outPktHdr);
								serverLocks->at(lockNum)->mailWaitQ->push(outMailHdr);
							} else {
								//Assign ownership of the lock and change state
								serverLocks->at(lockNum)->ownerMachineID = outPktHdr->to;
								serverLocks->at(lockNum)->ownerMailboxNum = outMailHdr->to;
								serverLocks->at(lockNum)->state = Busy;
								replyStream << -2;
							}
						}

						//Only sends a reply if we were able to acquire OR there was an error
						if (ifReply) {
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_Release: {
						DEBUG('S', "Message: Release\n");
						ss >> lockNum; //get lock ID
						DEBUG('T', "Release lock %s for machine %d, mailbox %d\n",
							  serverLocks->at(lockNum)->name.c_str(),
							  inPktHdr->from, inMailHdr->from);


						//Validate user input: send -1 if bad
						if (lockNum < 0 || lockNum >= serverLocks->size()) {
							replyStream << -1;
						} else {
							//Check whether or not we can release it
							if (serverLocks->at(lockNum) == NULL) {
								replyStream << -1;
							} else if (serverLocks->at(lockNum)->state == Available ||
									   serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to ||
									   serverLocks->at(lockNum)->ownerMailboxNum != outMailHdr->to) {
								replyStream << -1;
							} else {
								replyStream << -2;
								//Check if anyone is waiting so they must be woken up
								if (serverLocks->at(lockNum)->packetWaitQ->empty()) {
									serverLocks->at(lockNum)->state = Available;
									serverLocks->at(lockNum)->ownerMachineID = -1;
									serverLocks->at(lockNum)->ownerMailboxNum = -1;
								} else {
									//Change ownership and send message to waiting client
									PacketHeader *tempOutPktHdr = serverLocks->at(lockNum)->packetWaitQ->front();
									serverLocks->at(lockNum)->packetWaitQ->pop();
									MailHeader *tempOutMailHdr = serverLocks->at(lockNum)->mailWaitQ->front();
									serverLocks->at(lockNum)->mailWaitQ->pop();
									serverLocks->at(lockNum)->ownerMachineID = tempOutPktHdr->to;
									serverLocks->at(lockNum)->ownerMailboxNum = tempOutMailHdr->to;
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
						DEBUG('T', "Signal CV %s for machine %d, mailbox %d\n", serverCVs->at(cvNum)->name.c_str(),
							  inPktHdr->from, inMailHdr->from);

						//Validate user input: send -1 if bad
						if (lockNum < 0 || lockNum >= serverLocks->size() || cvNum < 0 || cvNum >= serverCVs->size()) {
							replyStream << -1;
						} else {
							//Do some more checks to ensure we can signal, like checking if the lock owner matches and the lock id matches the CV lock id
							if (serverLocks->at(lockNum) == NULL || serverCVs->at(cvNum) == NULL) {
								replyStream << -1;
							} else if (serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to ||
									   serverLocks->at(lockNum)->ownerMailboxNum != outMailHdr->to ||
									   serverCVs->at(cvNum)->lockID != lockNum) {
								replyStream << -1;
							} else {
								//If there is a waiting client, send reply so they can wake and go on to acquire
								if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
									replyStream << -1;
								} else {
									//Send message to waiting client
									replyStream << -2;
									PacketHeader *tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
									serverCVs->at(cvNum)->packetWaitQ->pop();
									MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
									serverCVs->at(cvNum)->mailWaitQ->pop();
									sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);

									if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
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
						DEBUG('T', "Wait on CV %s for machine %d, mailbox %d\n", serverCVs->at(cvNum)->name.c_str(),
							  inPktHdr->from, inMailHdr->from);

						bool ifReply = true;

						//Validate user input: send -1 if bad
						if (lockNum < 0 || lockNum >= serverLocks->size() || cvNum < 0 || cvNum >= serverCVs->size()) {
							replyStream << -1;
						} else {
							//Do some more checks to ensure we can wait
							if (serverLocks->at(lockNum) == NULL || serverCVs->at(cvNum) == NULL) {
								replyStream << -1;
							} else if (serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to ||
									   serverLocks->at(lockNum)->ownerMailboxNum != outMailHdr->to ||
									   (serverCVs->at(cvNum)->lockID != lockNum &&
										serverCVs->at(cvNum)->lockID != -1)) {
								//Enters this condition block if the lock owner does not match machine ID
								//And if the CV lock does not match lock ID and the lock is assigned
								//Which means it doesnt have index value of -1
								replyStream << -1;
							} else {
								ifReply = false;
								//If CV is unused, assign new lock
								if (serverCVs->at(cvNum)->lockID == -1) {
									serverCVs->at(cvNum)->lockID = lockNum;
								}
								serverCVs->at(cvNum)->packetWaitQ->push(outPktHdr);
								serverCVs->at(cvNum)->mailWaitQ->push(outMailHdr);

								//Change ownership and send message to waiting client
								PacketHeader *tempOutPktHdr = serverLocks->at(lockNum)->packetWaitQ->front();
								MailHeader *tempOutMailHdr = serverLocks->at(lockNum)->mailWaitQ->front();
								if (!(serverLocks->at(lockNum)->packetWaitQ->empty())) {
									serverLocks->at(lockNum)->packetWaitQ->pop();
									serverLocks->at(lockNum)->mailWaitQ->pop();
									serverLocks->at(lockNum)->ownerMachineID = tempOutPktHdr->to;
									serverLocks->at(lockNum)->ownerMailboxNum = tempOutMailHdr->to;
									replyStream << -2;
									sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
								}
								else {
									serverLocks->at(lockNum)->state = Available;
								}
							}
						}

						if (ifReply) {
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_Broadcast: {
						DEBUG('S', "Message: Broadcast\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						DEBUG('T', "Broadcast CV %s for machine %d, mailbox %d\n", serverCVs->at(cvNum)->name.c_str(),
							  inPktHdr->from, inMailHdr->from);

						//Validate user input: send -1 if bad
						if (lockNum < 0 || lockNum >= serverLocks->size() || cvNum < 0 || cvNum >= serverCVs->size()) {
							replyStream << -1;
						} else {
							//Do some more checks to ensure we can broadcast
							if (serverLocks->at(lockNum) == NULL || serverCVs->at(cvNum) == NULL) {
								replyStream << -1;
							} else if (serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to ||
									   serverLocks->at(lockNum)->ownerMailboxNum != outMailHdr->to ||
									   (serverCVs->at(cvNum)->lockID != lockNum &&
										serverCVs->at(cvNum)->lockID != -1)) {
								replyStream << -1;
							} else {
								//If there is a waiting client, send reply so they can wake and go on to acquire
								if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
									replyStream << -1;
								} else {
									//do a simple loop and wake everybody up by message
									while (!serverCVs->at(cvNum)->packetWaitQ->empty()) {
										replyStream << -2;
										PacketHeader *tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
										serverCVs->at(cvNum)->packetWaitQ->pop();
										MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
										serverCVs->at(cvNum)->mailWaitQ->pop();
										sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
									}
									serverCVs->at(
											cvNum)->lockID = -1; //since we've woken everyone up, no one is waiting on the lock anymore
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
						DEBUG('T', "Creating MV %s for machine %d, mailbox %d\n", name.c_str(), inPktHdr->from,
							  inMailHdr->from);


						int existingMVID = -1;

						//Once again, we have to check to see if there's already a MV made with this name
						for (int i = 0; i < serverMVs->size(); i++) {
							if (name.compare(serverMVs->at(i)->name) == 0) {
								existingMVID = i;
								break;
							}
						}


						//If MV doesn't exist, create one
						if (existingMVID == -1) {

							//Create MV
							ServerMV *newMV = new ServerMV;
							newMV->name = name;
							newMV->vals = new int[mvSiz];
							newMV->length = mvSiz;
							for (int i = 0; i < mvSiz; i++) {
								newMV->vals[i] = 0;
							}
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
						DEBUG('T', "Set destroy MV %s for machine %d, mailbox %d\n", serverMVs->at(mvNum)->name.c_str(),
							  inPktHdr->from, inMailHdr->from);


						//Validate user input: send -1 if bad
						if (mvNum < 0 || mvNum >= serverMVs->size()) {
							replyStream << -1;
						} else {
							//Do one more check before destroying
							if (serverMVs->at(mvNum) == NULL) {
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
						DEBUG('T', "Set MV %s at postition %d to %d for machine %d, mailbox %d\n",
							  serverMVs->at(mvNum)->name.c_str(),
							  mvPos, mvVal, inPktHdr->from, inMailHdr->from);


						//Validate user input: send -1 if bad
						if (mvNum < 0 || mvNum >= serverMVs->size() || mvPos < 0) {
							replyStream << -1;
						} else {
							//Do some more checks before setting value
							if (serverMVs->at(mvNum) == NULL) {
								replyStream << -1;
							} else if (mvPos >= serverMVs->at(mvNum)->length) {
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
						DEBUG('T', "Get MV %s at postition %d for machine %d, mailbox %d\n",
							  serverMVs->at(mvNum)->name.c_str(),
							  mvPos, inPktHdr->from, inMailHdr->from);

						//Validate user input: send -1 if bad
						if (mvNum < 0 || mvNum >= serverMVs->size() || mvPos < 0) {
							replyStream << -1;
						} else {
							//Do some more checks before setting value
							if (serverMVs->at(mvNum) == NULL) {
								replyStream << -1;
							} else if (mvPos >= serverMVs->at(mvNum)->length) {
								replyStream << -1;
							} else {
								replyStream << serverMVs->at(mvNum)->vals[mvPos];
							}
						}
						sendReply(outPktHdr, outMailHdr, replyStream);
						break;
					}
					default:
						cout << "Unkonwn message type. Ignoring.\n";
						continue;
						break;
				}
			}
			else { //Handle a server request


			}



			//cout<<serverLocks->at(0)->packetWaitQ->size()<<"\n";


		}

	}
	else{

		//Initialization
		vector<ServerLock *> *serverLocks = new vector<ServerLock *>;
		vector<ServerCV *> *serverCVs = new vector<ServerCV *>;
		vector<ServerMV *> *serverMVs = new vector<ServerMV *>;
		vector<ServerRequest *> *serverRQs = new vector<ServerRequest *>;

		cout << "Running server\n";
		cout << "Set to handle " << NUM_SERVERS << " servers.\n";

		//uniqueID generator for shared variables
		int uniqueID = myMachineID * 100;

		while (true) {

			//Vars for holding message details
			PacketHeader *outPktHdr = new PacketHeader();
			PacketHeader *inPktHdr = new PacketHeader();
			MailHeader *outMailHdr = new MailHeader();
			MailHeader *inMailHdr = new MailHeader();
			char buffer[MaxMailSize];

			// Wait for message from a client machine
			postOffice->Receive(0, inPktHdr, inMailHdr, buffer);
			DEBUG('S', "Got \"%s\" from %d, box %d\n", buffer, inPktHdr->from, inMailHdr->from);

			// Send acknowledgement to the other machine (using "reply to" mailbox
			// in the message that just arrived
			outPktHdr->to = inPktHdr->from;
			outMailHdr->to = inMailHdr->from;
			outMailHdr->from = myMachineID;
			//outMailHdr->from = 0; //We don't want to send back a different reply mailbox

			//Pull out message type
			int type;
			stringstream ss; //new one every time to be safe
			ss << buffer;
			ss >> type;

			//Decide what to do based on message type
			string name;
			int lockNum, cvNum, mvSiz, mvNum, mvPos, mvVal;
			int requestID, machineID, mailbox, arg1, arg2, arg3, reply;
			stringstream replyStream;
			//First if statement does server vs. client request code
			if (type % 100 == type) {

				switch (type) {
					case SC_CreateLock: {
						DEBUG('S', "Message: Create lock\n");
						ss.get();
						getline(ss, name, '@'); //get name of lock
						DEBUG('T', "Creating lock %s for machine %d, mailbox %d\n", name.c_str(), inPktHdr->from,
							  inMailHdr->from);


						//Check to see if that lock exists already
						int existingLockID = -1;
						for (int i = 0; i < serverLocks->size(); i++) {
							if (name.compare(serverLocks->at(i)->name) == 0) {
								existingLockID = i;
								break;
							}
						}

						//If doesn't, check to see if it exists on other servers before making
						if (existingLockID == -1) {
							NewServerRequest(serverRQs, name, SC_Server_CreateLock, inPktHdr->from, inMailHdr->from, 0,
											 0,
											 0);
						} else { //Lock does exist, so just give its ID
							replyStream << existingLockID + uniqueID;
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_DestroyLock: {
						DEBUG('S', "Message: Destroy lock\n");
						ss >> lockNum; //get lock ID
						DEBUG('T', "Set destroy lock %s for machine %d, mailbox %d\n",
							  serverLocks->at(lockNum)->name.c_str(), inPktHdr->from, inMailHdr->from);

						if (lockNum / 100 != myMachineID) {
							NewServerRequest(serverRQs, "", SC_Server_DestroyLock, inPktHdr->from, inMailHdr->from,
											 lockNum, 0, 0);
						} else {
							lockNum = lockNum % 100;
							//Validate user input: send -1 if bad
							if (lockNum < 0 || lockNum >= serverLocks->size()) {
								replyStream << -1;
							} else {
								//Validate whether or not the lock exists
								if (serverLocks->at(lockNum) == NULL) {
									replyStream << -1;
								} else {
									serverLocks->at(lockNum)->isToBeDeleted = true;
									replyStream << -2;
								}
							}
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_CreateCondition: {
						DEBUG('S', "Message: Create condition\n");
						ss.get();
						getline(ss, name, '@'); //get name of CV
						DEBUG('T', "Creating CV %s for machine %d, mailbox %d\n", name.c_str(), inPktHdr->from,
							  inMailHdr->from);


						int existingCVID = -1;

						//Once again, we have to check to see if there's already a CV made with this name
						for (int i = 0; i < serverCVs->size(); i++) {
							if (name.compare(serverCVs->at(i)->name) == 0) {
								existingCVID = i;
								break;
							}
						}


						//If CV doesn't exist, make a new one
						if (existingCVID == -1) {
							NewServerRequest(serverRQs, name, SC_Server_CreateCondition, inPktHdr->from,
											 inMailHdr->from, 0,
											 0, 0);
						}
						else { //CV does exist, so give its ID
							replyStream << existingCVID + uniqueID;
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_DestroyCondition: {
						DEBUG('S', "Message: Destroy Condition\n");
						ss >> cvNum; //get lock ID
						DEBUG('T', "Set destroy CV %s for machine %d, mailbox %d\n", serverCVs->at(cvNum)->name.c_str(),
							  inPktHdr->from, inMailHdr->from);

						if (cvNum / 100 != myMachineID) {
							NewServerRequest(serverRQs, "", SC_Server_DestroyCondition, inPktHdr->from, inMailHdr->from,
											 cvNum, 0, 0);
						} else {
							cvNum = cvNum % 100;
							//Validate user input: send -1 if bad
							if (cvNum < 0 || cvNum >= serverCVs->size()) {
								replyStream << -1;
							} else {
								//Validate whether or not the CV exists
								if (serverCVs->at(cvNum) == NULL) {
									replyStream << -1;
								} else {
									serverCVs->at(cvNum)->isToBeDeleted = true;
									replyStream << -2;
								}
							}
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_Acquire: {
						DEBUG('S', "Message: Acquire\n");
						ss >> lockNum; //get lock ID
						/*DEBUG('T', "Acquire lock %s for machine %d, mailbox %d\n", serverLocks->at(lockNum)->name.c_str(),
                              inPktHdr->from, inMailHdr->from);*/

						if (lockNum / 100 != myMachineID) {
							NewServerRequest(serverRQs, "", SC_Server_Acquire, inPktHdr->from, inMailHdr->from, lockNum,
											 0, 0);
						} else {
							lockNum = lockNum % 100;
							bool ifReply = true;

							//Validate user input: send -1 if bad
							if (lockNum < 0 || lockNum >= serverLocks->size()) {
								replyStream << -1;
							} else {

								DEBUG('T', "Acquire lock %s for machine %d, mailbox %d\n",
									  serverLocks->at(lockNum)->name.c_str(),
									  inPktHdr->from, inMailHdr->from);

								//Check whether or not we can acquire it
								if (serverLocks->at(lockNum) == NULL) {
									replyStream << -1;
								} else if (serverLocks->at(lockNum)->ownerMachineID == outPktHdr->to &&
										   serverLocks->at(lockNum)->ownerMailboxNum == outMailHdr->to &&
										   serverLocks->at(lockNum)->state == Busy) {
									//TODO add check int he else if above to make sure not just ownerMachineID, but some kind of
									//TODO thread-specific id matches
									replyStream << -1;
								} else if (serverLocks->at(lockNum)->state == Busy) {
									//Go on the wait queue
									ifReply = false;
									serverLocks->at(lockNum)->packetWaitQ->push(outPktHdr);
									serverLocks->at(lockNum)->mailWaitQ->push(outMailHdr);
								} else {
									//Assign ownership of the lock and change state
									serverLocks->at(lockNum)->ownerMachineID = outPktHdr->to;
									serverLocks->at(lockNum)->ownerMailboxNum = outMailHdr->to;
									serverLocks->at(lockNum)->state = Busy;
									replyStream << -2;
								}
							}

							//Only sends a reply if we were able to acquire OR there was an error
							if (ifReply) {
								sendReply(outPktHdr, outMailHdr, replyStream);
							}
						}
						break;
					}
					case SC_Release: {
						DEBUG('S', "Message: Release\n");
						ss >> lockNum; //get lock ID
						/*DEBUG('T', "Release lock %s for machine %d, mailbox %d\n", serverLocks->at(lockNum)->name.c_str(),
                              inPktHdr->from, inMailHdr->from);*/

						if (lockNum / 100 != myMachineID) {
							NewServerRequest(serverRQs, "", SC_Server_Release, inPktHdr->from, inMailHdr->from, lockNum,
											 0, 0);
						} else {
							lockNum = lockNum % 100;
							//Validate user input: send -1 if bad
							if (lockNum < 0 || lockNum >= serverLocks->size()) {
								replyStream << -1;
							} else {

								DEBUG('T', "Release lock %s for machine %d, mailbox %d\n",
									  serverLocks->at(lockNum)->name.c_str(),
									  inPktHdr->from, inMailHdr->from);

								//Check whether or not we can release it
								if (serverLocks->at(lockNum) == NULL) {
									replyStream << -1;
								} else if (serverLocks->at(lockNum)->state == Available ||
										   serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to ||
										   serverLocks->at(lockNum)->ownerMailboxNum != outMailHdr->to) {
									replyStream << -1;
								} else {
									replyStream << -2;
									//Check if anyone is waiting so they must be woken up
									if (serverLocks->at(lockNum)->packetWaitQ->empty()) {
										serverLocks->at(lockNum)->state = Available;
										serverLocks->at(lockNum)->ownerMachineID = -1;
										serverLocks->at(lockNum)->ownerMailboxNum = -1;
									} else {
										//Change ownership and send message to waiting client
										PacketHeader *tempOutPktHdr = serverLocks->at(lockNum)->packetWaitQ->front();
										serverLocks->at(lockNum)->packetWaitQ->pop();
										MailHeader *tempOutMailHdr = serverLocks->at(lockNum)->mailWaitQ->front();
										serverLocks->at(lockNum)->mailWaitQ->pop();
										serverLocks->at(lockNum)->ownerMachineID = tempOutPktHdr->to;
										serverLocks->at(lockNum)->ownerMailboxNum = tempOutMailHdr->to;
										sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
									}
								}
							}
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_Signal: {
						DEBUG('S', "Message: Signal\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						/*DEBUG('T', "Signal CV %s for machine %d, mailbox %d\n", serverCVs->at(cvNum)->name.c_str(),
                              inPktHdr->from, inMailHdr->from);*/

						bool ifEnter = checkIfEnter(serverRQs, serverLocks, serverCVs, cvNum, lockNum, inPktHdr->from,
													inMailHdr->from, SC_Server_Signal1, 13);
						//if both exist, we will enter in

						if (ifEnter) {
							int lockID = lockNum;
							lockNum = lockNum % 100;
							cvNum = cvNum % 100;

							//Validate user input: send -1 if bad
							if (lockNum < 0 || lockNum >= serverLocks->size() || cvNum < 0 ||
								cvNum >= serverCVs->size()) {
								replyStream << -1;
							} else {

								DEBUG('T', "Signal CV %s for machine %d, mailbox %d\n",
									  serverCVs->at(cvNum)->name.c_str(),
									  inPktHdr->from, inMailHdr->from);

								//Do some more checks to ensure we can signal, like checking if the lock owner matches and the lock id matches the CV lock id
								if (serverLocks->at(lockNum) == NULL || serverCVs->at(cvNum) == NULL) {
									replyStream << -1;
								} else if (serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to ||
										   serverLocks->at(lockNum)->ownerMailboxNum != outMailHdr->to ||
										   serverCVs->at(cvNum)->lockID != lockID) {
									replyStream << -1;
								} else {
									//If there is a waiting client, send reply so they can wake and go on to acquire
									if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
										replyStream << -1;
									} else {
										//Send message to waiting client
										replyStream << -2;
										PacketHeader *tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
										serverCVs->at(cvNum)->packetWaitQ->pop();
										MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
										serverCVs->at(cvNum)->mailWaitQ->pop();
										sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);

										if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
											serverCVs->at(cvNum)->lockID = -1;
										}
									}
								}
							}
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_Wait: {
						DEBUG('S', "Message: Wait\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						/*DEBUG('T', "Wait on CV %s for machine %d, mailbox %d\n", serverCVs->at(cvNum)->name.c_str(),
                              inPktHdr->from, inMailHdr->from);*/

						bool ifEnter = checkIfEnter(serverRQs, serverLocks, serverCVs, cvNum, lockNum, inPktHdr->from,
													inMailHdr->from, SC_Server_Wait1, 16);

						if (ifEnter) {
							int lockID = lockNum;
							lockNum = lockNum % 100;
							cvNum = cvNum % 100;
							bool ifReply = true;

							//Validate user input: send -1 if bad
							if (lockNum < 0 || lockNum >= serverLocks->size() || cvNum < 0 ||
								cvNum >= serverCVs->size()) {
								replyStream << -1;
							} else {
								DEBUG('T', "Wait on CV %s for machine %d, mailbox %d\n",
									  serverCVs->at(cvNum)->name.c_str(),
									  inPktHdr->from, inMailHdr->from);
								//Do some more checks to ensure we can wait
								if (serverLocks->at(lockNum) == NULL || serverCVs->at(cvNum) == NULL) {
									replyStream << -1;
								} else if (serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to ||
										   serverLocks->at(lockNum)->ownerMailboxNum != outMailHdr->to ||
										   (serverCVs->at(cvNum)->lockID != lockID &&
											serverCVs->at(cvNum)->lockID != -1)) {
									//Enters this condition block if the lock owner does not match machine ID
									//And if the CV lock does not match lock ID and the lock is assigned
									//Which means it doesnt have index value of -1
									replyStream << -1;
								} else {
									ifReply = false;
									//If CV is unused, assign new lock
									if (serverCVs->at(cvNum)->lockID == -1) {
										serverCVs->at(cvNum)->lockID = lockID;
									}
									serverCVs->at(cvNum)->packetWaitQ->push(outPktHdr);
									serverCVs->at(cvNum)->mailWaitQ->push(outMailHdr);

									//Change ownership and send message to waiting client
									PacketHeader *tempOutPktHdr = serverLocks->at(lockNum)->packetWaitQ->front();
									MailHeader *tempOutMailHdr = serverLocks->at(lockNum)->mailWaitQ->front();

									if (!(serverLocks->at(lockNum)->packetWaitQ->empty())) {
										serverLocks->at(lockNum)->packetWaitQ->pop();
										serverLocks->at(lockNum)->mailWaitQ->pop();
										serverLocks->at(lockNum)->ownerMachineID = tempOutPktHdr->to;
										serverLocks->at(lockNum)->ownerMailboxNum = tempOutMailHdr->to;
										replyStream << -2;
										sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
									}
									else {
										serverLocks->at(lockNum)->state = Available;
									}
								}
							}

							if (ifReply) {
								sendReply(outPktHdr, outMailHdr, replyStream);
							}
						}
						break;
					}
					case SC_Broadcast: {
						DEBUG('S', "Message: Broadcast\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						/*DEBUG('T', "Broadcast CV %s for machine %d, mailbox %d\n", serverCVs->at(cvNum)->name.c_str(),
                              inPktHdr->from, inMailHdr->from);*/

						bool ifEnter = checkIfEnter(serverRQs, serverLocks, serverCVs, cvNum, lockNum, inPktHdr->from,
													inMailHdr->from, SC_Server_Broadcast1, 16);

						if (ifEnter) {
							lockNum = lockNum % 100;
							cvNum = cvNum % 100;
							//Validate user input: send -1 if bad
							if (lockNum < 0 || lockNum >= serverLocks->size() || cvNum < 0 ||
								cvNum >= serverCVs->size()) {
								replyStream << -1;
							} else {

								DEBUG('T', "Broadcast CV %s for machine %d, mailbox %d\n",
									  serverCVs->at(cvNum)->name.c_str(),
									  inPktHdr->from, inMailHdr->from);

								//Do some more checks to ensure we can broadcast
								if (serverLocks->at(lockNum) == NULL || serverCVs->at(cvNum) == NULL) {
									replyStream << -1;
								} else if (serverLocks->at(lockNum)->ownerMachineID != outPktHdr->to ||
										   serverLocks->at(lockNum)->ownerMailboxNum != outMailHdr->to ||
										   (serverCVs->at(cvNum)->lockID != lockNum &&
											serverCVs->at(cvNum)->lockID != -1)) {
									replyStream << -1;
								} else {
									//If there is a waiting client, send reply so they can wake and go on to acquire
									if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
										replyStream << -1;
									} else {
										//do a simple loop and wake everybody up by message
										while (!serverCVs->at(cvNum)->packetWaitQ->empty()) {
											replyStream << -2;
											PacketHeader *tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
											serverCVs->at(cvNum)->packetWaitQ->pop();
											MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
											serverCVs->at(cvNum)->mailWaitQ->pop();
											sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
										}
										serverCVs->at(
												cvNum)->lockID = -1; //since we've woken everyone up, no one is waiting on the lock anymore
									}
								}
							}
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_CreateMV: {
						DEBUG('S', "Message: CreateMV\n");
						ss.get();
						getline(ss, name, '@'); //get name of lock
						ss >> mvSiz;
						DEBUG('T', "Creating MV %s for machine %d, mailbox %d\n", name.c_str(), inPktHdr->from,
							  inMailHdr->from);


						int existingMVID = -1;

						//Once again, we have to check to see if there's already a MV made with this name
						for (int i = 0; i < serverMVs->size(); i++) {
							if (name.compare(serverMVs->at(i)->name) == 0) {
								existingMVID = i;
								break;
							}
						}


						//If MV doesn't exist, create one
						if (existingMVID == -1) {
							NewServerRequest(serverRQs, name, SC_Server_CreateMV, inPktHdr->from, inMailHdr->from,
											 mvSiz, 0,
											 0);
						}
						else {//MV Does exist, so return ID
							replyStream << existingMVID + uniqueID;
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_DestroyMV: {
						DEBUG('S', "Message: DestroyMV\n");
						ss >> mvNum;
						DEBUG('T', "Set destroy MV %s for machine %d, mailbox %d\n", serverMVs->at(mvNum)->name.c_str(),
							  inPktHdr->from, inMailHdr->from);

						if (mvNum / 100 != myMachineID) {
							NewServerRequest(serverRQs, "", SC_Server_DestroyMV, inPktHdr->from, inMailHdr->from, mvNum,
											 0, 0);
						} else {
							mvNum = mvNum % 100;
							//Validate user input: send -1 if bad
							if (mvNum < 0 || mvNum >= serverMVs->size()) {
								replyStream << -1;
							} else {
								//Do one more check before destroying
								if (serverMVs->at(mvNum) == NULL) {
									replyStream << -1;
								} else {
									serverMVs->at(mvNum)->isToBeDeleted = true;
									replyStream << -2;
								}
							}
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_SetMV: {
						DEBUG('S', "Message: SetMV\n");
						ss >> mvNum >> mvPos >> mvVal;


						if (mvNum / 100 != myMachineID) {
							NewServerRequest(serverRQs, "", SC_Server_SetMV, inPktHdr->from, inMailHdr->from, mvNum,
											 mvPos, mvVal);
						} else {
							mvNum = mvNum % 100;
							//Validate user input: send -1 if bad
							if (mvNum < 0 || mvNum >= serverMVs->size() || mvPos < 0) {
								replyStream << -1;
							} else {
								//Do some more checks before setting value
								if (serverMVs->at(mvNum) == NULL) {
									replyStream << -1;
								} else if (mvPos >= serverMVs->at(mvNum)->length) {
									replyStream << -1;
								} else {
									DEBUG('T', "Set MV %s at postition %d to %d for machine %d, mailbox %d\n",
										  serverMVs->at(mvNum)->name.c_str(),
										  mvPos, mvVal, inPktHdr->from, inMailHdr->from);
									serverMVs->at(mvNum)->vals[mvPos] = mvVal;
									replyStream << -2;
								}
							}
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					case SC_GetMV: {
						DEBUG('S', "Message: GetMV\n");
						ss >> mvNum >> mvPos;


						if (mvNum / 100 != myMachineID) {
							NewServerRequest(serverRQs, "", SC_Server_GetMV, inPktHdr->from, inMailHdr->from, mvNum,
											 mvPos, 0);
						} else {
							DEBUG('T', "Get MV %s at postition %d for machine %d, mailbox %d\n",
								  serverMVs->at(mvNum)->name.c_str(),
								  mvPos, inPktHdr->from, inMailHdr->from);
							mvNum = mvNum % 100;
							//Validate user input: send -1 if bad
							if (mvNum < 0 || mvNum >= serverMVs->size() || mvPos < 0) {
								replyStream << -1;
							} else {
								//Do some more checks before setting value
								if (serverMVs->at(mvNum) == NULL) {
									replyStream << -1;
								} else if (mvPos >= serverMVs->at(mvNum)->length) {
									replyStream << -1;
								} else {
									replyStream << serverMVs->at(mvNum)->vals[mvPos];
								}
							}
							sendReply(outPktHdr, outMailHdr, replyStream);
						}
						break;
					}
					default:
						cout << "Unkonwn message type. Ignoring.\n";
						continue;
						break;
				}
			}
			else if (type / 100 == 1) { //Handle a server request
				//pull some info right away since these are required by all requests
				ss >> requestID;
				ss >> machineID;
				ss >> mailbox;

				switch (type) {
					case SC_Server_CreateLock: {
						DEBUG('S', "Message: Server Create lock\n");
						ss.get();
						getline(ss, name, '@'); //get name of lock
						DEBUG('T', "SR from %d: Create lock request %s for client %d, mailbox %d\n", inPktHdr->from,
							  name.c_str(), machineID,
							  mailbox);

						//Check to see if that lock exists already
						int existingLockID = -1;
						for (int i = 0; i < serverLocks->size(); i++) {
							if (name.compare(serverLocks->at(i)->name) == 0) {
								existingLockID = i;
								break;
							}
						}

						//If doesn't, send a reply of NO to server that made the request
						if (existingLockID == -1) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_CreateLock, requestID, machineID,
											  mailbox, 0);
						}
						else { //Lock does exist, so just give its ID to the client and reply with yes to server making request
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_CreateLock, requestID, machineID,
											  mailbox, 1);
							sendReplyToClient(machineID, mailbox, existingLockID + uniqueID);
						}
						break;
					}
					case SC_Server_DestroyLock: {
						DEBUG('S', "Message: Destroy lock\n");
						ss >> lockNum; //get lock ID
						DEBUG('T', "SR from %d: Set destroy lock %d for machine %d, mailbox %d\n", inPktHdr->from,
							  lockNum,
							  machineID, mailbox);

						//If it's not in our indexes, we don't have it, so reply no
						if (lockNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_DestroyLock, requestID, machineID,
											  mailbox, 0);
						} else {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_DestroyLock, requestID, machineID,
											  mailbox, 1);
							lockNum = lockNum % 100; //grab lock index

							if (lockNum < 0 || lockNum >= serverLocks->size()) {
								sendReplyToClient(machineID, mailbox, -1);
							} else {
								//Validate whether or not the lock exists
								if (serverLocks->at(lockNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
								} else {
									serverLocks->at(lockNum)->isToBeDeleted = true;
									sendReplyToClient(machineID, mailbox, -2);
								}
							}
						}
						break;
					}
					case SC_Server_CreateCondition: {
						DEBUG('S', "Message: Server Create condition\n");
						ss.get();
						getline(ss, name, '@'); //get name of condition
						DEBUG('T', "SR from %d: Create condition request %s for client %d, mailbox %d\n",
							  inPktHdr->from,
							  name.c_str(), machineID,
							  mailbox);

						//Check to see if that condition exists already
						int existingCVID = -1;
						for (int i = 0; i < serverCVs->size(); i++) {
							if (name.compare(serverCVs->at(i)->name) == 0) {
								existingCVID = i;
								break;
							}
						}

						//If doesn't, send a reply of NO to server that made the request
						if (existingCVID == -1) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_CreateCondition, requestID,
											  machineID,
											  mailbox, 0);
						}
						else { //Condition does exist, so just give its ID to the client and reply with yes to server making request
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_CreateCondition, requestID,
											  machineID,
											  mailbox, 1);
							sendReplyToClient(machineID, mailbox, existingCVID + uniqueID);
						}
						break;
					}
					case SC_Server_DestroyCondition: {
						DEBUG('S', "Message: Destroy condition\n");
						ss >> cvNum; //get CV ID
						DEBUG('T', "SR from %d: Set destroy condition %d for machine %d, mailbox %d\n", inPktHdr->from,
							  lockNum, machineID, mailbox);

						//If it's not in our indexes, we don't have it, so reply no
						if (cvNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_DestroyCondition, requestID,
											  machineID,
											  mailbox, 0);
						} else {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_DestroyCondition, requestID,
											  machineID,
											  mailbox, 1);
							cvNum = cvNum % 100; //grab CV index

							if (cvNum < 0 || cvNum >= serverCVs->size()) {
								sendReplyToClient(machineID, mailbox, -1);
							} else {
								//Validate whether or not the CV exists
								if (serverCVs->at(cvNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
								} else {
									serverCVs->at(cvNum)->isToBeDeleted = true;
									sendReplyToClient(machineID, mailbox, -2);
								}
							}
						}
						break;
					}
					case SC_Server_Acquire: {
						DEBUG('S', "Message: Acquire\n");
						ss >> lockNum; //get lock ID
						DEBUG('T', "SR from %d: Acquire lock %d for machine %d, mailbox %d\n", inPktHdr->from, lockNum,
							  machineID, mailbox);

						//If it's not in our indexes, we don't have it, so reply no
						if (lockNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Acquire, requestID, machineID,
											  mailbox,
											  0);
						} else {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Acquire, requestID, machineID,
											  mailbox,
											  1);
							lockNum = lockNum % 100;

							//Validate user input: send 0 if bad
							if (lockNum < 0 || lockNum >= serverLocks->size()) {
								sendReplyToClient(machineID, mailbox, -1);
							} else {
								//Check whether or not we can acquire it
								if (serverLocks->at(lockNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
								} else if (serverLocks->at(lockNum)->ownerMachineID == machineID &&
										   serverLocks->at(lockNum)->ownerMailboxNum == mailbox &&
										   serverLocks->at(lockNum)->state == Busy) {
									//TODO add check int he else if above to make sure not just ownerMachineID, but some kind of
									//TODO thread-specific id matches
									sendReplyToClient(machineID, mailbox, -1);
								} else if (serverLocks->at(lockNum)->state == Busy) {
									//Go on the wait queue
									//Must create packets to store on queue for client
									PacketHeader *temp_outPktHdr = new PacketHeader();
									MailHeader *temp_outMailHdr = new MailHeader();

									temp_outPktHdr->to = machineID; //client machineID goes here
									temp_outMailHdr->to = mailbox; //client mailbox goes here
									temp_outMailHdr->from = myMachineID; //our server machineID goes here

									serverLocks->at(lockNum)->packetWaitQ->push(temp_outPktHdr);
									serverLocks->at(lockNum)->mailWaitQ->push(temp_outMailHdr);
								} else {
									//Assign ownership of the lock and change state
									serverLocks->at(lockNum)->ownerMachineID = machineID;
									serverLocks->at(lockNum)->ownerMailboxNum = mailbox;
									serverLocks->at(lockNum)->state = Busy;
									sendReplyToClient(machineID, mailbox, -2);
								}
							}
						}
						break;
					}
					case SC_Server_Release: {
						DEBUG('S', "Message: Release\n");
						ss >> lockNum; //get lock ID
						DEBUG('T', "SR from %d: Release lock %d for machine %d, mailbox %d\n", inPktHdr->from, lockNum,
							  machineID, mailbox);

						//If it's not in our indexes, we don't have it, so reply no
						if (lockNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Release, requestID, machineID,
											  mailbox, 0);
						} else {
							lockNum = lockNum % 100;
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Release, requestID, machineID,
											  mailbox, 1);

							//Check whether or not we can release it
							if (serverLocks->at(lockNum) == NULL) {
								sendReplyToClient(machineID, mailbox, -1);
							} else if (serverLocks->at(lockNum)->state == Available ||
									   serverLocks->at(lockNum)->ownerMachineID != machineID ||
									   serverLocks->at(lockNum)->ownerMailboxNum != mailbox) {
								sendReplyToClient(machineID, mailbox, -1);
							} else {
								//Check if anyone is waiting so they must be woken up
								if (serverLocks->at(lockNum)->packetWaitQ->empty()) {
									serverLocks->at(lockNum)->state = Available;
									serverLocks->at(lockNum)->ownerMachineID = -1;
									serverLocks->at(lockNum)->ownerMailboxNum = -1;
								} else {
									//Change ownership and send message to waiting client
									PacketHeader *tempOutPktHdr = serverLocks->at(lockNum)->packetWaitQ->front();
									serverLocks->at(lockNum)->packetWaitQ->pop();
									MailHeader *tempOutMailHdr = serverLocks->at(lockNum)->mailWaitQ->front();
									serverLocks->at(lockNum)->mailWaitQ->pop();
									serverLocks->at(lockNum)->ownerMachineID = tempOutPktHdr->to;
									serverLocks->at(lockNum)->ownerMailboxNum = tempOutMailHdr->to;
									sendReplyToClient(tempOutPktHdr->to, tempOutMailHdr->to, -2);
								}
								sendReplyToClient(machineID, mailbox, -2);
							}
						}
						break;
					}
					case SC_Server_Signal1: {
						DEBUG('S', "Message: Signal1\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						DEBUG('T', "SR from %d: Signal1 lock %d and CV %d for machine %d, mailbox %d\n", lockNum, cvNum,
							  inPktHdr->from, inMailHdr->from);

						//Case where lock is on server, but CV is on another server
						//If it's not in our indexes, we don't have it, so reply no
						if (cvNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID,
											  mailbox,
											  0);
						} else {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID,
											  mailbox,
											  1);
							cvNum = cvNum % 100; //grab CV index

							if (cvNum < 0 || cvNum >= serverCVs->size()) {
								sendReplyToClient(machineID, mailbox, -1);
							} else {
								//Validate whether or not the CV exists
								if (serverCVs->at(cvNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
								} else if (serverCVs->at(cvNum)->lockID == lockNum) {
									//If there is a waiting client, send reply so they can wake and go on to acquire
									if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
										sendReplyToClient(machineID, mailbox, -1);
									} else {
										//Send message to waiting client
										replyStream << -2;
										PacketHeader *tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
										serverCVs->at(cvNum)->packetWaitQ->pop();
										MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
										serverCVs->at(cvNum)->mailWaitQ->pop();
										sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);

										if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
											serverCVs->at(cvNum)->lockID = -1;
										}
										sendReplyToClient(machineID, mailbox, -2);
									}
								}
							}
						}
						break;
					}
					case SC_Server_Signal2: {
						DEBUG('S', "Message: Signal2\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						DEBUG('T', "SR from %d: Signal2 lock %d and CV %d for machine %d, mailbox %d\n", lockNum, cvNum,
							  inPktHdr->from, inMailHdr->from);

						//Case where CV is on server, but lock is on another server
						//If it's not in our indexes, we don't have it, so reply no
						//Replies: -1 (we don't do anything since lock is BAD), 0 is lock not found, 1 is lock found and GOOD
						if (lockNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal2, requestID, machineID,
											  mailbox,
											  0);
						} else {
							lockNum = lockNum % 100; //grab lock index

							if (lockNum < 0 || lockNum >= serverLocks->size()) {
								//-1 is a special type of YES, in that we don't do anything since lock is bad
								sendReplyToClient(machineID, mailbox, -1);
								sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal2, requestID, machineID,
												  mailbox, -1);
							} else {
								//Validate whether or not the lock exists
								if (serverLocks->at(lockNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal2, requestID,
													  machineID,
													  mailbox, -1);
								} else if (serverLocks->at(lockNum)->ownerMachineID != machineID ||
										   serverLocks->at(lockNum)->ownerMailboxNum != mailbox) {
									//check if client actually owns the lock
									sendReplyToClient(machineID, mailbox, -1);
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal2, requestID,
													  machineID,
													  mailbox, -1);
								} else {
									//Client owns lock, so send YES (1) to server, but have the server also perform action to avoid race conditions
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal2, requestID,
													  machineID,
													  mailbox, 1);
								}
							}
						}
						break;
					}
					case SC_Server_Signal3: {
						DEBUG('S', "Message: Signal3\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						DEBUG('T', "SR from %d: Signal3 lock %d and CV %d for machine %d, mailbox %d\n", lockNum, cvNum,
							  inPktHdr->from, inMailHdr->from);
						//Case where CV and lock is on another server

						//bools check whether lock state
						bool foundLock = false;
						bool badLock = false;

						//may be used for later in CV checking for matching lockID
						int lockID = lockNum;

						//If it's not in our indexes, we don't have it, so reply no
						if (lockNum / 100 != myMachineID) {
							foundLock = false;
						} else { //we found the lock, so lets check if it's good
							lockNum = lockNum % 100; //grab lock index
							foundLock = true;

							if (lockNum < 0 || lockNum >= serverLocks->size()) {
								sendReplyToClient(machineID, mailbox, -1);
								badLock = true;
								//-1 is a special type of YES, in that we don't do anything
								//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, -1);
							} else {
								//Validate whether or not the lock exists
								if (serverLocks->at(lockNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
									badLock = true;
									//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, -1);
								} else if (serverLocks->at(lockNum)->ownerMachineID != machineID ||
										   serverLocks->at(lockNum)->ownerMailboxNum != mailbox) {
									//check if client actually owns the lock
									sendReplyToClient(machineID, mailbox, -1);
									badLock = true;
									//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, -1);
								} else {
									//badLock = false; lock is good!
									//Send YES to server, but have the server also perform action to avoid race conditions
									//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, 1);
								}
							}
						}
						//reply cases: -1 (bad lock or bad CV so stop searching), 0 (no lock nor cv), 1 (both lock and CV), 2(lock no CV), 3 (CV no lock)
						//if we found the lock and it was bad, send -1 as a special YES case (meaning to stop searching such the lock is already invalid)
						if (foundLock && badLock) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID, machineID,
											  mailbox,
											  -1);
						} else { //if we did not have to stop searching (lock may or may not exist but not a BAD lock), check if the CV exists on the server as well

							//If it's not in our indexes, we don't have it, so reply no
							if (cvNum / 100 != myMachineID) {
								if (foundLock && !badLock) { //if we found the lock but not the CV, reply with 2
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID,
													  machineID,
													  mailbox, 2);
								} else if (!foundLock) { //if we didn't find the lock or CV, reply with 0
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID,
													  machineID,
													  mailbox, 0);
								}
							} else {
								//we found the CV, so lets check lock ID
								cvNum = cvNum % 100; //grab CV index

								if (cvNum < 0 || cvNum >= serverCVs->size()) {
									sendReplyToClient(machineID, mailbox, -1);
								} else {
									//Validate whether or not the CV exists
									if (serverCVs->at(cvNum) == NULL) {
										//reply server with -1 to stop searching since CV is bad
										sendReplyToClient(machineID, mailbox, -1);
										sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID,
														  machineID, mailbox, -1);
									} else if (serverCVs->at(cvNum)->lockID ==
											   lockID) { //if CV exists, check to see if lockID matches with lock provided by client

										if (lockID / 100 ==
											myMachineID) { //if the lock exists on the server, go ahead and do everything
											//If there is a waiting client, send reply so they can wake and go on to acquire
											if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
												//if there is noone to signal, bad CV so reply with -1 to server since we are done
												sendReplyToClient(machineID, mailbox, -1);
												sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3,
																  requestID,
																  machineID, mailbox, -1);
											} else {
												//Send message to waiting client
												replyStream << -2;
												PacketHeader *tempOutPktHdr = serverCVs->at(
														cvNum)->packetWaitQ->front();
												serverCVs->at(cvNum)->packetWaitQ->pop();
												MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
												serverCVs->at(cvNum)->mailWaitQ->pop();
												sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);

												if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
													serverCVs->at(cvNum)->lockID = -1;
												}

												//reply with 1 to server since we have both lock and CV
												sendReplyToClient(machineID, mailbox, -2);
												sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3,
																  requestID,
																  machineID, mailbox, 1);
											}
										} else {
											//if the lock doesn't exist on the server
											//we need to ping server and see whether or not the ownerMachineId matches our lock's owner
											//at this point, we know that the CV and lock match, just need to verify owner of the lock
											sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID,
															  machineID, mailbox, 3);
										}
									} else {
										//if the lock ID does not match CV lock id, bad CV so reply with -1 to server since we are done
										sendReplyToClient(machineID, mailbox, -1);
										sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID,
														  machineID, mailbox, -1);
									}
								}
							}
						}
						break;
					}
					case SC_Server_Wait1: {
						DEBUG('S', "Message: Wait1\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						DEBUG('T', "SR from %d: Wait1 CV %d for machine %d, mailbox %d\n", cvNum,
							  inPktHdr->from, inMailHdr->from);

						//Case where lock is on server, but CV is on another server
						//Replies: -1 we stop looking since CV is bad, 1 is we have the CV, 0 is we do not have the CV.
						//If it's not in our indexes, we don't have it, so reply no
						if (cvNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait1, requestID, machineID,
											  mailbox,
											  0);
						} else {
							cvNum = cvNum % 100;
							//Validate user input: send -1 if bad
							if (cvNum < 0 || cvNum >= serverCVs->size()) {
								sendReplyToClient(machineID, mailbox, -1);
								sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait1, requestID, machineID,
												  mailbox, -1);
							} else {
								//Do some more checks to ensure we can wait
								if (serverCVs->at(cvNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait1, requestID, machineID,
													  mailbox, -1);
								} else if (serverCVs->at(cvNum)->lockID != lockNum &&
										   serverCVs->at(cvNum)->lockID != -1) {
									//Enters this condition block if the lock owner does not match machine ID
									//And if the CV lock does not match lock ID and the lock is assigned
									//Which means it doesnt have index value of -1
									sendReplyToClient(machineID, mailbox, -1);
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait1, requestID, machineID,
													  mailbox, -1);
								} else {
									//If CV is unused, assign new lock
									if (serverCVs->at(cvNum)->lockID == -1) {
										serverCVs->at(cvNum)->lockID = lockNum;
									}

									//Create headers for pushing client on waitqueue for CV
									PacketHeader *temp_outPktHdr = new PacketHeader();
									MailHeader *temp_outMailHdr = new MailHeader();

									temp_outPktHdr->to = machineID; //client machineID goes here
									temp_outMailHdr->to = mailbox; //client mailbox
									temp_outMailHdr->from = myMachineID; //our server machineID goes here

									//Push client info onto wait queue and change lock ownership on server
									serverCVs->at(cvNum)->packetWaitQ->push(temp_outPktHdr);
									serverCVs->at(cvNum)->mailWaitQ->push(temp_outMailHdr);

									//send reply to server where ownership of the lock will be changed
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait1, requestID, machineID,
													  mailbox, 1);
								}
							}
						}
						break;
					}
					case SC_Server_Wait2: {
						DEBUG('S', "Message: Wait2\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						DEBUG('T', "SR from %d: Wait2 CV %d for machine %d, mailbox %d\n", cvNum,
							  inPktHdr->from, inMailHdr->from);

						//Case where CV is on server, but lock is on another server
						//Replies: -1 we stop looking since lock is bad, 1 is we have the lock, 0 is we do not have the lock.
						//If it's not in our indexes, we don't have it, so reply no
						if (lockNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait2, requestID, machineID,
											  mailbox,
											  0);
						} else {
							int lockID = lockNum;
							lockNum = lockNum % 100;
							//Validate user input: send -1 if bad
							if (lockNum < 0 || lockNum >= serverLocks->size()) {
								sendReplyToClient(machineID, mailbox, -1);
								sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait2, requestID, machineID,
												  mailbox, -1);
							} else {
								//Do some more checks to ensure we can wait
								if (serverLocks->at(lockNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait2, requestID, machineID,
													  mailbox, -1);
								} else if (serverLocks->at(lockNum)->ownerMachineID != machineID ||
										   serverLocks->at(lockNum)->ownerMailboxNum != mailbox) {
									//Enters this condition block if the lock owner does not match machine ID
									//Which means it doesnt have index value of -1
									sendReplyToClient(machineID, mailbox, -1);
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait2, requestID, machineID,
													  mailbox, -1);
								} else {

									//Change ownership of lock and send message to waiting client
									PacketHeader *tempOutPktHdr = serverLocks->at(lockNum)->packetWaitQ->front();
									MailHeader *tempOutMailHdr = serverLocks->at(lockNum)->mailWaitQ->front();

									if (!(serverLocks->at(lockNum)->packetWaitQ->empty())) {
										serverLocks->at(lockNum)->packetWaitQ->pop();
										serverLocks->at(lockNum)->mailWaitQ->pop();
										serverLocks->at(lockNum)->ownerMachineID = tempOutPktHdr->to;
										serverLocks->at(lockNum)->ownerMailboxNum = tempOutMailHdr->to;
										replyStream << -2;
										sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
									}
									else {
										serverLocks->at(lockNum)->state = Available;
									}

									//send reply to server where CV will change lockID and push client onto wait queue
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait2, requestID, machineID,
													  mailbox, 1);
								}
							}
						}
						break;
					}
					case SC_Server_Wait3: {
						DEBUG('S', "Message: Wait3\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						DEBUG('T', "SR from %d: Wait3 CV %d for machine %d, mailbox %d\n", cvNum,
							  inPktHdr->from, inMailHdr->from);

						//bools check lock state
						bool foundLock = false;
						bool badLock = false;

						//may be used for later in CV checking for matching lockID
						int lockID = lockNum;

						//If it's not in our indexes, we don't have it, so reply no
						if (lockNum / 100 != myMachineID) {
							foundLock = false;
						} else { //we found the lock, so lets check if it's good
							lockNum = lockNum % 100; //grab lock index
							foundLock = true;

							if (lockNum < 0 || lockNum >= serverLocks->size()) {
								sendReplyToClient(machineID, mailbox, -1);
								badLock = true;
								//-1 is a special type of YES, in that we don't do anything
								//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, -1);
							} else {
								//Validate whether or not the lock exists
								if (serverLocks->at(lockNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
									badLock = true;
									//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, -1);
								} else if (serverLocks->at(lockNum)->ownerMachineID != machineID ||
										   serverLocks->at(lockNum)->ownerMailboxNum != mailbox) {
									//check if client actually owns the lock
									sendReplyToClient(machineID, mailbox, -1);
									badLock = true;
									//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, -1);
								} else {
									//badLock = false; lock is good!
									//Send YES to server, but have the server also perform action to avoid race conditions
									//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, 1);
								}
							}
						}
						//reply cases: -1 (bad lock or bad CV so stop searching), 0 (no lock nor cv), 1 (both lock and CV), 2(lock no CV), 3 (CV no lock)
						//if we found the lock and it was bad, send -1 as a special YES case (meaning to stop searching such the lock is already invalid)
						if (foundLock && badLock) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID, machineID,
											  mailbox,
											  -1);
						} else { //if we did not have to stop searching (lock may or may not exist but not a BAD lock), check if the CV exists on the server as well

							//If it's not in our indexes, we don't have it, so reply no
							if (cvNum / 100 != myMachineID) {
								if (foundLock && !badLock) { //if we found the lock but not the CV, reply with 2
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID,
													  machineID,
													  mailbox, 2);
								} else if (!foundLock) { //if we didn't find the lock or CV, reply with 0
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID,
													  machineID,
													  mailbox, 0);
								}
							} else {
								//we found the CV, so lets check lock ID
								cvNum = cvNum % 100; //grab CV index

								if (cvNum < 0 || cvNum >= serverCVs->size()) {
									sendReplyToClient(machineID, mailbox, -1);
								} else {
									//Validate whether or not the CV exists
									if (serverCVs->at(cvNum) == NULL) {
										//reply server with -1 to stop searching since CV is bad
										sendReplyToClient(machineID, mailbox, -1);
										sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID,
														  machineID, mailbox, -1);
									} else if (serverCVs->at(cvNum)->lockID != lockNum &&
											   serverCVs->at(cvNum)->lockID != -1) {
										sendReplyToClient(machineID, mailbox, -1);
										sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID,
														  machineID, mailbox, -1);
									} else { //if CV exists, check to see if lock is on the server

										if (lockID / 100 ==
											myMachineID) { //if the lock exists on the server, go ahead and do everything

											//If CV is unused, assign new lock
											if (serverCVs->at(cvNum)->lockID == -1) {
												serverCVs->at(cvNum)->lockID = lockID;
											}

											//Create headers for pushing client on waitqueue for CV
											PacketHeader *temp_outPktHdr = new PacketHeader();
											MailHeader *temp_outMailHdr = new MailHeader();

											temp_outPktHdr->to = machineID; //client machineID goes here
											temp_outMailHdr->to = mailbox; //client mailbox
											temp_outMailHdr->from = myMachineID; //our server machineID goes here

											//Push client info onto wait queue and change lock ownership on server
											serverCVs->at(cvNum)->packetWaitQ->push(temp_outPktHdr);
											serverCVs->at(cvNum)->mailWaitQ->push(temp_outMailHdr);

											//Change ownership and send message to waiting client
											PacketHeader *tempOutPktHdr = serverLocks->at(
													lockNum)->packetWaitQ->front();
											MailHeader *tempOutMailHdr = serverLocks->at(lockNum)->mailWaitQ->front();

											if (!(serverLocks->at(lockNum)->packetWaitQ->empty())) {
												serverLocks->at(lockNum)->packetWaitQ->pop();
												serverLocks->at(lockNum)->mailWaitQ->pop();
												serverLocks->at(lockNum)->ownerMachineID = tempOutPktHdr->to;
												serverLocks->at(lockNum)->ownerMailboxNum = tempOutMailHdr->to;
												replyStream << -2;
												sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
											}
											else {
												serverLocks->at(lockNum)->state = Available;
											}

											//send reply to server and confirm everything has been done already
											sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait1, requestID,
															  machineID, mailbox, 1);
										} else {
											//if the lock doesn't exist on the server
											//we need to ping server and see whether or not the ownerMachineId matches our lock's owner
											//at this point, we know that the CV and lock match, just need to verify owner of the lock
											sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal3, requestID,
															  machineID, mailbox, 3);
										}
									}
								}
							}
						}
						break;
					}
					case SC_Server_Broadcast1: {
						DEBUG('S', "Message: Broadcast1\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						DEBUG('T', "SR from %d: Broadcast1 lock %d and CV %d for machine %d, mailbox %d\n", lockNum, cvNum,
							  inPktHdr->from, inMailHdr->from);

						//Case where lock is on server, but CV is on another server
						//If it's not in our indexes, we don't have it, so reply no
						if (cvNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast1, requestID, machineID,
											  mailbox,
											  0);
						} else {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast1, requestID, machineID,
											  mailbox,
											  1);
							cvNum = cvNum % 100; //grab CV index

							if (cvNum < 0 || cvNum >= serverCVs->size()) {
								sendReplyToClient(machineID, mailbox, -1);
							} else {
								//Validate whether or not the CV exists
								if (serverCVs->at(cvNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
								} else if (serverCVs->at(cvNum)->lockID == lockNum) {
									//If there is a waiting client, send reply so they can wake and go on to acquire
									if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
										sendReplyToClient(machineID, mailbox, -1);
									} else {

										//If there is a waiting client, send reply so they can wake and go on to acquire
										if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
											sendReplyToClient(machineID, mailbox, -1);
										} else {
											//do a simple loop and wake everybody up by message
											replyStream << -2;
											while (!serverCVs->at(cvNum)->packetWaitQ->empty()) {
												PacketHeader *tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
												serverCVs->at(cvNum)->packetWaitQ->pop();
												MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
												serverCVs->at(cvNum)->mailWaitQ->pop();
												sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
											}
											serverCVs->at(
													cvNum)->lockID = -1; //since we've woken everyone up, no one is waiting on the lock anymore
											sendReplyToClient(machineID, mailbox, -2);										
										}
									}
								}
							}
						}
						break;

					}
					case SC_Server_Broadcast2: {
						DEBUG('S', "Message: Broadcast2\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						DEBUG('T', "SR from %d: Broadcast2 lock %d and CV %d for machine %d, mailbox %d\n", lockNum, cvNum,
							  inPktHdr->from, inMailHdr->from);

						//Case where CV is on server, but lock is on another server
						//If it's not in our indexes, we don't have it, so reply no
						//Replies: -1 (we don't do anything since lock is BAD), 0 is lock not found, 1 is lock found and GOOD
						if (lockNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast2, requestID, machineID,
											  mailbox,
											  0);
						} else {
							lockNum = lockNum % 100; //grab lock index

							if (lockNum < 0 || lockNum >= serverLocks->size()) {
								//-1 is a special type of YES, in that we don't do anything since lock is bad
								sendReplyToClient(machineID, mailbox, -1);
								sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast2, requestID, machineID,
												  mailbox, -1);
							} else {
								//Validate whether or not the lock exists
								if (serverLocks->at(lockNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast2, requestID,
													  machineID,
													  mailbox, -1);
								} else if (serverLocks->at(lockNum)->ownerMachineID != machineID ||
										   serverLocks->at(lockNum)->ownerMailboxNum != mailbox) {
									//check if client actually owns the lock
									sendReplyToClient(machineID, mailbox, -1);
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast2, requestID,
													  machineID,
													  mailbox, -1);
								} else {
									//Client owns lock, so send YES (1) to server, but have the server also perform action to avoid race conditions
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast2, requestID,
													  machineID,
													  mailbox, 1);
								}
							}
						}
						break;					
					}
					case SC_Server_Broadcast3: {
						DEBUG('S', "Message: Signal3\n");
						ss >> cvNum >> lockNum; //get lock and CV num
						DEBUG('T', "SR from %d: Signal3 lock %d and CV %d for machine %d, mailbox %d\n", lockNum, cvNum,
							  inPktHdr->from, inMailHdr->from);
						//Case where CV and lock is on another server

						//bools check whether lock state
						bool foundLock = false;
						bool badLock = false;

						//may be used for later in CV checking for matching lockID
						int lockID = lockNum;

						//If it's not in our indexes, we don't have it, so reply no
						if (lockNum / 100 != myMachineID) {
							foundLock = false;
						} else { //we found the lock, so lets check if it's good
							lockNum = lockNum % 100; //grab lock index
							foundLock = true;

							if (lockNum < 0 || lockNum >= serverLocks->size()) {
								sendReplyToClient(machineID, mailbox, -1);
								badLock = true;
								//-1 is a special type of YES, in that we don't do anything
								//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, -1);
							} else {
								//Validate whether or not the lock exists
								if (serverLocks->at(lockNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
									badLock = true;
									//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, -1);
								} else if (serverLocks->at(lockNum)->ownerMachineID != machineID ||
										   serverLocks->at(lockNum)->ownerMailboxNum != mailbox) {
									//check if client actually owns the lock
									sendReplyToClient(machineID, mailbox, -1);
									badLock = true;
									//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, -1);
								} else {
									//badLock = false; lock is good!
									//Send YES to server, but have the server also perform action to avoid race conditions
									//sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Signal1, requestID, machineID, mailbox, 1);
								}
							}
						}
						//reply cases: -1 (bad lock or bad CV so stop searching), 0 (no lock nor cv), 1 (both lock and CV), 2(lock no CV), 3 (CV no lock)
						//if we found the lock and it was bad, send -1 as a special YES case (meaning to stop searching such the lock is already invalid)
						if (foundLock && badLock) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast3, requestID, machineID,
											  mailbox,
											  -1);
						} else { //if we did not have to stop searching (lock may or may not exist but not a BAD lock), check if the CV exists on the server as well

							//If it's not in our indexes, we don't have it, so reply no
							if (cvNum / 100 != myMachineID) {
								if (foundLock && !badLock) { //if we found the lock but not the CV, reply with 2
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast3, requestID,
													  machineID,
													  mailbox, 2);
								} else if (!foundLock) { //if we didn't find the lock or CV, reply with 0
									sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast3, requestID,
													  machineID,
													  mailbox, 0);
								}
							} else {
								//we found the CV, so lets check lock ID
								cvNum = cvNum % 100; //grab CV index

								if (cvNum < 0 || cvNum >= serverCVs->size()) {
									sendReplyToClient(machineID, mailbox, -1);
								} else {
									//Validate whether or not the CV exists
									if (serverCVs->at(cvNum) == NULL) {
										//reply server with -1 to stop searching since CV is bad
										sendReplyToClient(machineID, mailbox, -1);
										sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast3, requestID,
														  machineID, mailbox, -1);
									} else if (serverCVs->at(cvNum)->lockID ==
											   lockID) { //if CV exists, check to see if lockID matches with lock provided by client

										if (lockID / 100 ==
											myMachineID) { //if the lock exists on the server, go ahead and do everything
											//If there is a waiting client, send reply so they can wake and go on to acquire
											if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
												//if there is noone to signal, bad CV so reply with -1 to server since we are done
												sendReplyToClient(machineID, mailbox, -1);
												sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast3,
																  requestID,
																  machineID, mailbox, -1);
											} else {
												//do a simple loop and wake everybody up by message
												replyStream << -2;
												while (!serverCVs->at(cvNum)->packetWaitQ->empty()) {
													PacketHeader *tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
													serverCVs->at(cvNum)->packetWaitQ->pop();
													MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
													serverCVs->at(cvNum)->mailWaitQ->pop();
													sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
												}
												serverCVs->at(
														cvNum)->lockID = -1; //since we've woken everyone up, no one is waiting on the lock anymore


												//reply with 1 to server since we have both lock and CV
												sendReplyToClient(machineID, mailbox, -2);
												sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast3,
																  requestID,
																  machineID, mailbox, 1);
											}
										} else {
											//if the lock doesn't exist on the server
											//we need to ping server and see whether or not the ownerMachineId matches our lock's owner
											//at this point, we know that the CV and lock match, just need to verify owner of the lock
											sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast3, requestID,
															  machineID, mailbox, 3);
										}
									} else {
										//if the lock ID does not match CV lock id, bad CV so reply with -1 to server since we are done
										sendReplyToClient(machineID, mailbox, -1);
										sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Broadcast3, requestID,
														  machineID, mailbox, -1);
									}
								}
							}
						}
						break;
					}
					case SC_Server_CreateMV: {
						DEBUG('S', "Message: Server Create monitor variable\n");
						ss.get();
						getline(ss, name, '@'); //get name of mv
						DEBUG('T', "SR from %d: Create monitor variable request %s for client %d, mailbox %d\n",
							  inPktHdr->from, name.c_str(), machineID,
							  mailbox);

						//Check to see if that mv exists already
						int existingMVID = -1;
						for (int i = 0; i < serverMVs->size(); i++) {
							if (name.compare(serverMVs->at(i)->name) == 0) {
								existingMVID = i;
								break;
							}
						}

						//If doesn't, send a reply of NO to server that made the request
						if (existingMVID == -1) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_CreateMV, requestID, machineID,
											  mailbox,
											  0);
						}
						else { //Monitor variable does exist, so just give its ID to the client and reply with yes to server making request
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_CreateMV, requestID, machineID,
											  mailbox,
											  1);
							sendReplyToClient(machineID, mailbox, existingMVID + uniqueID);
						}
						break;
					}
					case SC_Server_DestroyMV: {
						DEBUG('S', "Message: Destroy monitor variable\n");
						ss >> mvNum; //get MV ID
						DEBUG('T', "SR from %d: Set destroy monitor variable %d for machine %d, mailbox %d\n",
							  inPktHdr->from, mvNum, machineID, mailbox);

						//If it's not in our indexes, we don't have it, so reply no
						if (mvNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_DestroyMV, requestID, machineID,
											  mailbox, 0);
						} else {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_DestroyMV, requestID, machineID,
											  mailbox, 1);
							mvNum = mvNum % 100; //grab MV index

							if (mvNum < 0 || mvNum >= serverMVs->size()) {
								sendReplyToClient(machineID, mailbox, -1);
							} else {
								//Validate whether or not the MV exists
								if (serverMVs->at(mvNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
								} else {
									serverMVs->at(mvNum)->isToBeDeleted = true;
									sendReplyToClient(machineID, mailbox, -2);
								}
							}
						}
						break;
					}
					case SC_Server_SetMV: {
						DEBUG('S', "Message: SetMV\n");
						ss >> mvNum >> mvPos >> mvVal;
						DEBUG('T', "SR from %d: SetMV %d at postition %d to %d for machine %d, mailbox %d\n",
							  inPktHdr->from, mvNum,
							  mvPos, mvVal, machineID, mailbox);

						//If it's not in our indexes, we don't have it, so reply no
						if (mvNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_SetMV, requestID, machineID,
											  mailbox,
											  0);
						} else {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_SetMV, requestID, machineID,
											  mailbox,
											  1);
							mvNum = mvNum % 100; //grab MV index

							if (mvNum < 0 || mvNum >= serverMVs->size() || mvPos < 0) {
								sendReplyToClient(machineID, mailbox, -1);
							} else {
								//Do some more checks before setting value
								if (serverMVs->at(mvNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
								} else if (mvPos >= serverMVs->at(mvNum)->length) {
									sendReplyToClient(machineID, mailbox, -1);
								} else {
									serverMVs->at(mvNum)->vals[mvPos] = mvVal;
									sendReplyToClient(machineID, mailbox, -2);
								}
							}
						}
						break;
					}
					case SC_Server_GetMV: {
						DEBUG('S', "Message: GetMV\n");
						ss >> mvNum >> mvPos;
						DEBUG('T', "SR from %d: GetMV %d at postition %d to %d for machine %d, mailbox %d\n",
							  inPktHdr->from, mvNum,
							  mvPos, mvVal, machineID, mailbox);

						//If it's not in our indexes, we don't have it, so reply no
						if (mvNum / 100 != myMachineID) {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_GetMV, requestID, machineID,
											  mailbox,
											  0);
						} else {
							sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_GetMV, requestID, machineID,
											  mailbox,
											  1);
							mvNum = mvNum % 100; //grab MV index

							if (mvNum < 0 || mvNum >= serverMVs->size() || mvPos < 0) {
								sendReplyToClient(machineID, mailbox, -1);
							} else {
								//Do some more checks before getting value
								if (serverMVs->at(mvNum) == NULL) {
									sendReplyToClient(machineID, mailbox, -1);
								} else if (mvPos >= serverMVs->at(mvNum)->length) {
									sendReplyToClient(machineID, mailbox, -1);
								} else {
									sendReplyToClient(machineID, mailbox, serverMVs->at(mvNum)->vals[mvPos]);
								}
							}
						}
						break;
					}
					default:
						cout << "Unkonwn message type. Ignoring.\n";
						continue;
						break;
				}

			} else { //handle a server reply to one of our requests
				//pull some info right away since these are required by all replies
				ss >> requestID;
				ss >> machineID;
				ss >> mailbox;
				ss >> reply;

				//will hold data about the current request
				ServerRequest *currentRequest;
				bool yes;

				//populate data regarding current server request
				if (requestID != -1) {
					currentRequest = serverRQs->at(requestID);
					yes = currentRequest->yes;
				}

				switch (type) {
					case SC_ServerReply_CreateLock: {
						DEBUG('S', "Message: Server Reply Create lock\n");
						DEBUG('T', "SR from %d: Create lock reply %s for client %d, mailbox %d\n", inPktHdr->from,
							  currentRequest->name.c_str(), machineID,
							  mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									cout << "Creating Lock\n";
									ServerLock *newLock = new ServerLock;
									newLock->name = currentRequest->name;
									newLock->packetWaitQ = new queue<PacketHeader *>();
									newLock->mailWaitQ = new queue<MailHeader *>();
									newLock->state = Available;
									newLock->isToBeDeleted = false;
									newLock->ownerMachineID = -1;
									newLock->ownerMailboxNum = -1;

									//Add to vector
									serverLocks->push_back(newLock);

									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, (serverLocks->size() - 1) + uniqueID);
								} else {
									currentRequest->noCount++;
								}
							} else {
								currentRequest->yes = true;
								cout << "Other server %d already has lock.\n";
							}
						}
						break;
					}
					case SC_ServerReply_DestroyLock: {
						DEBUG('S', "Message: Server Reply Destroy lock\n");
						DEBUG('T', "SR from %d: Set destroy lock reply for machine %d, mailbox %d\n", inPktHdr->from,
							  machineID, mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "Lock doesn't exist to destroy it\n";
								} else {
									currentRequest->noCount++;
								}
							} else {
								cout << "Other server destroyed lock\n";
								currentRequest->yes = true;
							}
						}
						break;
					}
					case SC_ServerReply_CreateCondition: {
						DEBUG('S', "Message: Server Reply Create condition\n");
						DEBUG('T', "SR from %d: Create condition reply %s for client %d, mailbox %d\n", inPktHdr->from,
							  currentRequest->name.c_str(), machineID,
							  mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									DEBUG('S', "Creating Condition\n");
									//Create Condition
									ServerCV *newCV = new ServerCV;
									newCV->name = currentRequest->name;
									newCV->packetWaitQ = new queue<PacketHeader *>();
									newCV->mailWaitQ = new queue<MailHeader *>();
									newCV->isToBeDeleted = false;
									newCV->lockID = -1;

									//Add to vector
									serverCVs->push_back(newCV);

									//Send reply - copy this template
									sendReplyToClient(machineID, mailbox, (serverCVs->size() - 1) + uniqueID);
								} else {
									currentRequest->noCount++;
								}
							} else {
								currentRequest->yes = true;
								DEBUG('T', "Other server %d already has condition.\n", inPktHdr->from);
							}
						}
						break;
					}
					case SC_ServerReply_DestroyCondition: {
						DEBUG('S', "Message: Server Reply Destroy condition\n");
						DEBUG('T', "SR from %d: Set destroy condition reply for machine %d, mailbox %d\n",
							  inPktHdr->from,
							  machineID, mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									DEBUG('S', "Condition doesn't exist to destroy it\n");
								} else {
									currentRequest->noCount++;
								}
							} else {
								currentRequest->yes = true;
								DEBUG('T', "Other server destroyed condition\n");
							}
						}
						break;
					}
					case SC_ServerReply_Acquire: {
						DEBUG('S', "Message: Reply Acquire\n");
						DEBUG('T', "SR from %d: Acquire lock reply %d for machine %d, mailbox %d\n", inPktHdr->from,
							  currentRequest->arg1,
							  machineID, mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "Lock doesn't exist to acquire it\n";
								} else {
									currentRequest->noCount++;
								}
							} else {
								currentRequest->yes = true;
								cout << "Other server %d acquired lock\n";
							}
						}
						break;
					}
					case SC_ServerReply_Release: {
						DEBUG('S', "Message: Reply Release\n");
						DEBUG('T', "SR from %d: Release lock reply %d for machine %d, mailbox %d\n", inPktHdr->from,
							  currentRequest->arg1,
							  machineID, mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "Lock doesn't exist to release it\n";
								} else {
									currentRequest->noCount++;
								}
							} else {
								currentRequest->yes = true;
								cout << "Other server %d released lock\n";
							}
						}
						break;
					}
					case SC_ServerReply_Signal1: {
						DEBUG('S', "Message: Reply Signal1\n");
						DEBUG('T', "SR from %d: Signal1 reply of %d for machine %d, mailbox %d\n", inPktHdr->from,
							  reply,
							  machineID, mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "Condition doesn't exist to signal it\n";
								} else {
									currentRequest->noCount++;
								}
							} else {
								currentRequest->yes = true;
								cout << "Other server %d signaled condition\n";
							}
						}
						break;
					}
					case SC_ServerReply_Signal2: {
						DEBUG('S', "Message: Reply Signal2\n");
						DEBUG('T', "SR from %d: Signal2 reply of %d for machine %d, mailbox %d\n", inPktHdr->from,
							  reply,
							  machineID, mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "Condition doesn't exist to signal it\n";
								} else {
									currentRequest->noCount++;
								}
							} else if (reply == -1) {
								currentRequest->yes = true;
								sendReplyToClient(machineID, mailbox, -1);
								cout << "Condition is bad, can't signal it\n";
							} else if (reply == 1) {
								currentRequest->yes = true;
								cvNum = currentRequest->arg1 % 100;

								//We do not need to perform any more validations since we did so in SC_Signal through checkIfEnter
								//If there is a waiting client, send reply so they can wake and go on to acquire
								if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
									sendReplyToClient(machineID, mailbox, -1);
								} else {
									//Send message to waiting client
									replyStream << -2;
									PacketHeader *tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
									serverCVs->at(cvNum)->packetWaitQ->pop();
									MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
									serverCVs->at(cvNum)->mailWaitQ->pop();
									sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);

									if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
										serverCVs->at(cvNum)->lockID = -1;
									}
									sendReplyToClient(machineID, mailbox, -2);
								}
								cout << "Condition was signaled by us since lock exists\n";
							}
						}
						break;
					}
					case SC_ServerReply_Signal3: {
						DEBUG('S', "Message: Reply Signal3\n");
						DEBUG('T', "SR from %d: Signal3 reply of %d for machine %d, mailbox %d\n", inPktHdr->from,
							  reply,
							  machineID, mailbox);

						//reply cases:
						//-1 (bad lock or bad CV so stop searching)
						//0 (no lock nor cv)
						//1 (both lock and CV)
						//2 (lock no CV) --> Lock info is good:
						//							1) 3 is there, get server info and send request to handle action
						//							2) 3 is not there, just wait
						//3 (CV no lock) --> CV info is good:
						//							1) 2 is there, send request type to signal (no more validation needed)
						//							2) 2 is not there, store other server info and send request to handle

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1 ||
									(currentRequest->cvFound && currentRequest->noCount == NUM_SERVERS - 2) ||
									(currentRequest->lockFound && currentRequest->noCount == NUM_SERVERS - 2)) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "Lock or CV is missing or neither of them exist\n";
								} else {
									currentRequest->noCount++;
								}
							} else if (reply == -1) {
								currentRequest->yes = true;
								sendReplyToClient(machineID, mailbox, -1);
								cout << "Bad lock or CV\n";
							} else if (reply == 2) {
								currentRequest->lockFound = true;

								//if the CV has been found, let server with CV handle it
								if (currentRequest->cvFound) {
									sendReplyToServer(currentRequest->replyServerMachineID,
													  currentRequest->replyServerMailbox, SC_ServerReply_Signal4, -1,
													  machineID, mailbox, currentRequest->arg1);
									currentRequest->yes = true;
									cout << "Lock found and CV found\n";
								} else if (currentRequest->noCount == NUM_SERVERS - 2) {
									sendReplyToClient(machineID, mailbox, -1);
									currentRequest->yes = true;
									cout << "CV does not exist\n";
								}
							} else if (reply == 3) {
								currentRequest->cvFound = true;

								PacketHeader *temp_outPktHdr = new PacketHeader();
								MailHeader *temp_outMailHdr = new MailHeader();

								temp_outPktHdr->to = inPktHdr->from; //other server machineID goes here
								temp_outMailHdr->to = inMailHdr->from; //default mailbox is zero
								temp_outMailHdr->from = myMachineID; //our server machineID goes here

								currentRequest->replyServerMachineID = temp_outPktHdr;
								currentRequest->replyServerMailbox = temp_outMailHdr;

								//if the lock has been found, let server with CV handle it
								if (currentRequest->lockFound) {
									sendReplyToServer(currentRequest->replyServerMachineID,
													  currentRequest->replyServerMailbox, SC_ServerReply_Signal4, -1,
													  machineID, mailbox, currentRequest->arg1);
									currentRequest->yes = true;
									cout << "Lock found and CV found\n";
								} else if (currentRequest->noCount == NUM_SERVERS - 2) {
									sendReplyToClient(machineID, mailbox, -1);
									currentRequest->yes = true;
									cout << "Lock does not exist\n";
								}
							} else if (reply == 1) {
								currentRequest->yes = true;
								cout << "Wait completed on another server since both lock and cv were there\n";
							}
						}
						break;
					}
					case SC_ServerReply_Signal4: {
						DEBUG('S', "Message: Reply Signal4\n");
						DEBUG('T', "SR from %d: Signal4 reply of %d for machine %d, mailbox %d\n", inPktHdr->from,
							  reply,
							  machineID, mailbox);

						cvNum = reply % 100;

						//We do not need to perform any more validations since we did so in SC_Signal through checkIfEnter
						//If there is a waiting client, send reply so they can wake and go on to acquire
						if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
							sendReplyToClient(machineID, mailbox, -1);
						} else {
							//Send message to waiting client
							replyStream << -2;
							PacketHeader *tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
							serverCVs->at(cvNum)->packetWaitQ->pop();
							MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
							serverCVs->at(cvNum)->mailWaitQ->pop();
							sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);

							if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
								serverCVs->at(cvNum)->lockID = -1;
							}
							sendReplyToClient(machineID, mailbox, -2);
						}
						break;
					}
					case SC_ServerReply_Wait1: {
						DEBUG('S', "Message: Reply Wait1\n");
						DEBUG('T', "SR from %d: Wait1 reply %d for machine %d, mailbox %d\n", inPktHdr->from, reply,
							  machineID, mailbox);
						//case where lock is on server, but CV is on another server

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "CV does not exist\n";
								} else {
									currentRequest->noCount++;
								}
							} else if (reply == -1) {
								currentRequest->yes = true;
								cout << "CV info is bad\n";
							} else if (reply == 1) {
								lockNum = currentRequest->arg2 % 100; //grab lock index

								//Change ownership of lock and send message to waiting client
								PacketHeader *tempOutPktHdr = serverLocks->at(lockNum)->packetWaitQ->front();
								MailHeader *tempOutMailHdr = serverLocks->at(lockNum)->mailWaitQ->front();

								if (!(serverLocks->at(lockNum)->packetWaitQ->empty())) {
									serverLocks->at(lockNum)->packetWaitQ->pop();
									serverLocks->at(lockNum)->mailWaitQ->pop();
									serverLocks->at(lockNum)->ownerMachineID = tempOutPktHdr->to;
									serverLocks->at(lockNum)->ownerMailboxNum = tempOutMailHdr->to;
									replyStream << -2;
									sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
								}
								else {
									serverLocks->at(lockNum)->state = Available;
								}
								cout << "CV and Lock exist, we are now waiting\n";
							}
						}
						break;
					}
					case SC_ServerReply_Wait2: {
						DEBUG('S', "Message: Reply Wait2\n");
						DEBUG('T', "SR from %d: Wait2 reply %d for machine %d, mailbox %d\n", inPktHdr->from, reply,
							  machineID, mailbox);
						//case where CV is on server, but lock is on another server

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "Lock does not exist\n";
								} else {
									currentRequest->noCount++;
								}
							} else if (reply == -1) {
								currentRequest->yes = true;
								cout << "Lock info is bad\n";
							} else if (reply == 1) {
								cvNum = currentRequest->arg1 % 100; //grab CV index

								//If CV is unused, assign new lock
								if (serverCVs->at(cvNum)->lockID == -1) {
									serverCVs->at(cvNum)->lockID = currentRequest->arg2;
								}

								//Create headers for pushing client on waitqueue for CV
								PacketHeader *temp_outPktHdr = new PacketHeader();
								MailHeader *temp_outMailHdr = new MailHeader();

								temp_outPktHdr->to = machineID; //client machineID goes here
								temp_outMailHdr->to = mailbox; //client mailbox
								temp_outMailHdr->from = myMachineID; //our server machineID goes here

								//Push client info onto wait queue and change lock ownership on server
								serverCVs->at(cvNum)->packetWaitQ->push(temp_outPktHdr);
								serverCVs->at(cvNum)->mailWaitQ->push(temp_outMailHdr);
								cout << "Lock and CV exist, lock ownership changed, we are now waiting\n";
							}
						}
						break;
					}
					case SC_ServerReply_Wait3: {
						DEBUG('S', "Message: Reply Wait3\n");
						DEBUG('T', "SR from %d: Wait3 reply of %d for machine %d, mailbox %d\n", inPktHdr->from, reply,
							  machineID, mailbox);

						//reply cases:
						//-1 (bad lock or bad CV so stop searching)
						//0 (no lock nor cv)
						//1 (both lock and CV)
						//2 (lock no CV) --> Lock info is good:
						//							1) 3 is there, get server info and send request to handle action
						//							2) 3 is not there, just wait
						//3 (CV no lock) --> CV info is good:
						//							1) 2 is there, send request type to signal (no more validation needed)
						//							2) 2 is not there, store other server info and send request to handle

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1 ||
									(currentRequest->cvFound && currentRequest->noCount == NUM_SERVERS - 2) ||
									(currentRequest->lockFound && currentRequest->noCount == NUM_SERVERS - 2)) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "Lock or CV is missing or neither of them exist\n";
								} else {
									currentRequest->noCount++;
								}
							} else if (reply == -1) {
								currentRequest->yes = true;
								sendReplyToClient(machineID, mailbox, -1);
								cout << "Bad lock or CV\n";
							} else if (reply == 2) {
								currentRequest->lockFound = true;

								//store server info of lock
								PacketHeader *temp_outPktHdr = new PacketHeader();
								MailHeader *temp_outMailHdr = new MailHeader();

								temp_outPktHdr->to = inPktHdr->from; //other server machineID goes here
								temp_outMailHdr->to = inMailHdr->from; //default mailbox is zero
								temp_outMailHdr->from = myMachineID; //our server machineID goes here

								currentRequest->replyServerMachineID_two = temp_outPktHdr;
								currentRequest->replyServerMailbox_two = temp_outMailHdr;

								//if the CV has been found, let server with CV handle it
								if (currentRequest->cvFound) {
									sendReplyToServer(currentRequest->replyServerMachineID,
													  currentRequest->replyServerMailbox, SC_ServerReply_Wait4, -1,
													  machineID, mailbox,
													  currentRequest->arg1 * 1000000 + currentRequest->arg2);
									cout << "Lock found and CV found\n";
								} else if (currentRequest->noCount == NUM_SERVERS - 2) {
									sendReplyToClient(machineID, mailbox, -1);
									currentRequest->yes = true;
									cout << "CV not found\n";
								}
							} else if (reply == 3) {
								currentRequest->cvFound = true;

								//store server info of CV
								PacketHeader *temp_outPktHdr = new PacketHeader();
								MailHeader *temp_outMailHdr = new MailHeader();

								temp_outPktHdr->to = inPktHdr->from; //other server machineID goes here
								temp_outMailHdr->to = inMailHdr->from; //default mailbox is zero
								temp_outMailHdr->from = myMachineID; //our server machineID goes here

								currentRequest->replyServerMachineID = temp_outPktHdr;
								currentRequest->replyServerMailbox = temp_outMailHdr;

								//if the lock has been found, let server with CV handle it
								if (currentRequest->lockFound) {
									sendReplyToServer(currentRequest->replyServerMachineID,
													  currentRequest->replyServerMailbox, SC_ServerReply_Wait4, -1,
													  machineID, mailbox,
													  currentRequest->arg1 * 1000000 + currentRequest->arg2);
									cout << "Lock found and CV found\n";
								} else if (currentRequest->noCount == NUM_SERVERS - 2) {
									sendReplyToClient(machineID, mailbox, -1);
									currentRequest->yes = true;
									cout << "Lock not found\n";
								}
							} else if (reply == 1) {
								currentRequest->yes = true;
								cout << "Wait completed on another server since both lock and cv were there\n";
							} else if (reply == 4) {
								//Send lock and change ownership, thereby releasing it
								sendReplyToServer(currentRequest->replyServerMachineID,
												  currentRequest->replyServerMailbox,
												  SC_ServerReply_Wait5, -1, machineID, mailbox, currentRequest->arg2);
								currentRequest->yes = true;
								cout << "Change lock ownership then we are done\n";
							}
						}
						break;
					}
					case SC_ServerReply_Wait4: {
						DEBUG('S', "Message: Reply Wait4\n");
						DEBUG('T', "SR from %d: Wait4 reply of %d for machine %d, mailbox %d\n", inPktHdr->from, reply,
							  machineID, mailbox);

						cvNum = (reply / 100000000) % 100;
						lockNum = reply % 1000000;

						//If CV is unused, assign new lock
						if (serverCVs->at(cvNum)->lockID == -1) {
							serverCVs->at(cvNum)->lockID = lockNum;
						}

						//Create headers for pushing client on waitqueue for CV
						PacketHeader *temp_outPktHdr = new PacketHeader();
						MailHeader *temp_outMailHdr = new MailHeader();

						temp_outPktHdr->to = machineID; //client machineID goes here
						temp_outMailHdr->to = mailbox; //client mailbox
						temp_outMailHdr->from = myMachineID; //our server machineID goes here

						//Push client info onto wait queue and change lock ownership on server
						serverCVs->at(cvNum)->packetWaitQ->push(temp_outPktHdr);
						serverCVs->at(cvNum)->mailWaitQ->push(temp_outMailHdr);

						//send reply to server where ownership of the lock will be changed
						sendReplyToServer(outPktHdr, outMailHdr, SC_ServerReply_Wait3, requestID, machineID, mailbox,
										  4);

						break;
					}
					case SC_ServerReply_Wait5: {
						DEBUG('S', "Message: Reply Wait5\n");
						DEBUG('T', "SR from %d: Wait5 reply of %d for machine %d, mailbox %d\n", inPktHdr->from, reply,
							  machineID, mailbox);

						lockNum = reply % 100; //grab lock index

						//Change ownership of lock and send message to waiting client
						PacketHeader *tempOutPktHdr = serverLocks->at(lockNum)->packetWaitQ->front();
						MailHeader *tempOutMailHdr = serverLocks->at(lockNum)->mailWaitQ->front();

						if (!(serverLocks->at(lockNum)->packetWaitQ->empty())) {
							serverLocks->at(lockNum)->packetWaitQ->pop();
							serverLocks->at(lockNum)->mailWaitQ->pop();
							serverLocks->at(lockNum)->ownerMachineID = tempOutPktHdr->to;
							serverLocks->at(lockNum)->ownerMailboxNum = tempOutMailHdr->to;
							replyStream << -2;
							sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
						} else {
							serverLocks->at(lockNum)->state = Available;
						}

						sendReplyToClient(machineID, mailbox, -2);
						break;
					}
					case SC_ServerReply_Broadcast1: {
						DEBUG('S', "Message: Reply Broadcast1\n");
						DEBUG('T', "SR from %d: Broadcast1 reply of %d for machine %d, mailbox %d\n", inPktHdr->from,
							  reply,
							  machineID, mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "Condition doesn't exist to broadcast it\n";
								} else {
									currentRequest->noCount++;
								}
							} else {
								currentRequest->yes = true;
								cout << "Other server %d broadcasted condition\n";
							}
						}
						break;
					}
					case SC_ServerReply_Broadcast2: {
						DEBUG('S', "Message: Reply Broadcast2\n");
						DEBUG('T', "SR from %d: Broadcast2 reply of %d for machine %d, mailbox %d\n", inPktHdr->from,
							  reply,
							  machineID, mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "Condition doesn't exist to broadcast it\n";
								} else {
									currentRequest->noCount++;
								}
							} else if (reply == -1) {
								currentRequest->yes = true;
								sendReplyToClient(machineID, mailbox, -1);
								cout << "Condition is bad, can't broadcast it\n";
							} else if (reply == 1) {
								currentRequest->yes = true;
								cvNum = currentRequest->arg1 % 100;

								//We do not need to perform any more validations since we did so in SC_Signal through checkIfEnter
								//If there is a waiting client, send reply so they can wake and go on to acquire
								if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
									sendReplyToClient(machineID, mailbox, -1);
								} else {
									//do a simple loop and wake everybody up by message
									replyStream << -2;
									while (!serverCVs->at(cvNum)->packetWaitQ->empty()) {
										PacketHeader *tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
										serverCVs->at(cvNum)->packetWaitQ->pop();
										MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
										serverCVs->at(cvNum)->mailWaitQ->pop();
										sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
									}
									serverCVs->at(cvNum)->lockID = -1; //since we've woken everyone up, no one is waiting on the lock anymore
									sendReplyToClient(machineID, mailbox, -2);									
								}
								cout << "Condition was signaled by us since lock exists\n";
							}
						}
						break;
					}
					case SC_ServerReply_Broadcast3: {
						DEBUG('S', "Message: Reply Broadcast3\n");
						DEBUG('T', "SR from %d: Broadcast3 reply of %d for machine %d, mailbox %d\n", inPktHdr->from,
							  reply,
							  machineID, mailbox);

						//reply cases:
						//-1 (bad lock or bad CV so stop searching)
						//0 (no lock nor cv)
						//1 (both lock and CV)
						//2 (lock no CV) --> Lock info is good:
						//							1) 3 is there, get server info and send request to handle action
						//							2) 3 is not there, just wait
						//3 (CV no lock) --> CV info is good:
						//							1) 2 is there, send request type to signal (no more validation needed)
						//							2) 2 is not there, store other server info and send request to handle

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1 ||
									(currentRequest->cvFound && currentRequest->noCount == NUM_SERVERS - 2) ||
									(currentRequest->lockFound && currentRequest->noCount == NUM_SERVERS - 2)) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "Lock or CV is missing or neither of them exist\n";
								} else {
									currentRequest->noCount++;
								}
							} else if (reply == -1) {
								currentRequest->yes = true;
								sendReplyToClient(machineID, mailbox, -1);
								cout << "Bad lock or CV\n";
							} else if (reply == 2) {
								currentRequest->lockFound = true;

								//if the CV has been found, let server with CV handle it
								if (currentRequest->cvFound) {
									sendReplyToServer(currentRequest->replyServerMachineID,
													  currentRequest->replyServerMailbox, SC_ServerReply_Broadcast4, -1,
													  machineID, mailbox, currentRequest->arg1);
									currentRequest->yes = true;
									cout << "Lock found and CV found\n";
								} else if (currentRequest->noCount == NUM_SERVERS - 2) {
									sendReplyToClient(machineID, mailbox, -1);
									currentRequest->yes = true;
									cout << "CV does not exist\n";
								}
							} else if (reply == 3) {
								currentRequest->cvFound = true;

								PacketHeader *temp_outPktHdr = new PacketHeader();
								MailHeader *temp_outMailHdr = new MailHeader();

								temp_outPktHdr->to = inPktHdr->from; //other server machineID goes here
								temp_outMailHdr->to = inMailHdr->from; //default mailbox is zero
								temp_outMailHdr->from = myMachineID; //our server machineID goes here

								currentRequest->replyServerMachineID = temp_outPktHdr;
								currentRequest->replyServerMailbox = temp_outMailHdr;

								//if the lock has been found, let server with CV handle it
								if (currentRequest->lockFound) {
									sendReplyToServer(currentRequest->replyServerMachineID,
													  currentRequest->replyServerMailbox, SC_ServerReply_Broadcast4, -1,
													  machineID, mailbox, currentRequest->arg1);
									currentRequest->yes = true;
									cout << "Lock found and CV found\n";
								} else if (currentRequest->noCount == NUM_SERVERS - 2) {
									sendReplyToClient(machineID, mailbox, -1);
									currentRequest->yes = true;
									cout << "Lock does not exist\n";
								}
							} else if (reply == 1) {
								currentRequest->yes = true;
								cout << "Wait completed on another server since both lock and cv were there\n";
							}
						}
						break;
					}
					case SC_ServerReply_Broadcast4: {
						DEBUG('S', "Message: Reply Broadcast4\n");
						DEBUG('T', "SR from %d: Broadcast4 reply of %d for machine %d, mailbox %d\n", inPktHdr->from,
							  reply,
							  machineID, mailbox);

						cvNum = reply % 100;

						//We do not need to perform any more validations since we did so in SC_Signal through checkIfEnter
						//If there is a waiting client, send reply so they can wake and go on to acquire
						if (serverCVs->at(cvNum)->packetWaitQ->empty()) {
							sendReplyToClient(machineID, mailbox, -1);
						} else {
							//do a simple loop and wake everybody up by message
							replyStream << -2;
							while (!serverCVs->at(cvNum)->packetWaitQ->empty()) {
								PacketHeader *tempOutPktHdr = serverCVs->at(cvNum)->packetWaitQ->front();
								serverCVs->at(cvNum)->packetWaitQ->pop();
								MailHeader *tempOutMailHdr = serverCVs->at(cvNum)->mailWaitQ->front();
								serverCVs->at(cvNum)->mailWaitQ->pop();
								sendReply(tempOutPktHdr, tempOutMailHdr, replyStream);
							}
							serverCVs->at(cvNum)->lockID = -1; //since we've woken everyone up, no one is waiting on the lock anymore
							sendReplyToClient(machineID, mailbox, -2);		
						}
						break;
					}
					case SC_ServerReply_CreateMV: {
						DEBUG('S', "Message: Server Reply Create monitor variable\n");
						DEBUG('T', "SR from %d: Create monitor variable reply %s for client %d, mailbox %d\n",
							  inPktHdr->from, currentRequest->name.c_str(), machineID,
							  mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									mvSiz = currentRequest->arg1;

									//Create MV
									ServerMV *newMV = new ServerMV;
									newMV->name = currentRequest->name;
									newMV->vals = new int[mvSiz];
									newMV->length = mvSiz;
									for (int i = 0; i < mvSiz; i++) {
										newMV->vals[i] = 0;
									}
									newMV->isToBeDeleted = false;

									//Add to vector
									serverMVs->push_back(newMV);

									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, (serverMVs->size() - 1) + uniqueID);
									cout << "Creating MV\n";
								} else {
									currentRequest->noCount++;
								}
							} else {
								currentRequest->yes = true;
							}
						}
						break;
					}
					case SC_ServerReply_DestroyMV: {
						DEBUG('S', "Message: Server Reply Destroy monitor variable\n");
						DEBUG('T', "SR from %d: Destroy monitor variable reply for machine %d, mailbox %d\n",
							  inPktHdr->from, machineID, mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
								} else {
									currentRequest->noCount++;
								}
							} else {
								currentRequest->yes = true;
							}
						}
						break;
					}
					case SC_ServerReply_SetMV: {
						DEBUG('S', "Message: Server Reply Set monitor variable\n");
						DEBUG('T', "SR from %d: Set monitor variable reply for machine %d, mailbox %d\n",
							  inPktHdr->from,
							  machineID, mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "MV not found\n";
								} else {
									currentRequest->noCount++;
								}
							} else {
								currentRequest->yes = true;
								cout << "MV found\n";
							}
						}
						break;
					}
					case SC_ServerReply_GetMV: {
						DEBUG('S', "Message: Server Reply Get monitor variable\n");
						DEBUG('T', "SR from %d: Get monitor variable reply for machine %d, mailbox %d\n",
							  inPktHdr->from,
							  machineID, mailbox);

						if (!yes) {
							//if we got NO
							if (reply == 0) {
								currentRequest->noCount++;
								//if we got all our NO replies, perform action
								if (currentRequest->noCount == NUM_SERVERS - 1) {
									//Send reply
									currentRequest->yes = true;
									sendReplyToClient(machineID, mailbox, -1);
									cout << "MV not found\n";
								} else {
									currentRequest->noCount++;
								}
							} else {
								currentRequest->yes = true;
								cout << "MV found\n";
							}
						}
						break;
					}
					default:
						cout << "Unkonwn message type. Ignoring.\n";
						continue;
						break;
				}
			}

			//cout<<serverLocks->at(0)->packetWaitQ->size()<<"\n";


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
