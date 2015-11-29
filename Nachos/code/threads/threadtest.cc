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
void sendReplyToServer(PacketHeader* outPktHdr, MailHeader* outMailHdr, int requestType, int requestID, int machineID, int mailbox, int reply) {
	stringstream replyStream; 
	replyStream << requestType << requestID << machineID << mailbox << reply;
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

	//send serverRequest to all other servers
	for(int i = 0; i < NUM_SERVERS; i++) {
		//if not my machineID, send request to this machineID
		if(i != myMachineID) {
			//create packets to send to server
			PacketHeader* outPktHdr = new PacketHeader(); 
			MailHeader* outMailHdr = new MailHeader();
			
			outPktHdr->to = i; //other server machineID goes here
			outMailHdr->to = 0; //default mailbox is zero
			outMailHdr->from = myMachineID; //our server machineID goes here

			//create stringstream object to store data, use an if statement to send different data depending on what the syscall is
			stringstream ss;
			
			if(requestType == SC_Server_CreateCondition || requestType == SC_Server_CreateLock || requestType == SC_Server_CreateMV) {
				ss << sr->requestType << " " << sr->requestID << " " << sr->machineID << " " << sr->mailbox << " " << sr->name;
			} else if(requestType == SC_Server_Acquire || requestType == SC_Server_Release || requestType == SC_Server_DestroyLock || requestType == SC_Server_DestroyCondition || requestType == SC_Server_DestroyMV) {
				ss << sr->requestType << " " << sr->requestID << " " << sr->machineID << " " << sr->mailbox << " " << sr->arg1; 
			} else if(requestType == SC_Server_Wait || requestType == SC_Server_Signal || requestType == SC_Server_Broadcast) {
				ss << sr->requestType << " " << sr->requestID << " " << sr->machineID << " " << sr->mailbox << " " << sr->arg1 << " " << sr->arg2;
			} else {
				ss << sr->requestType << " " << sr->requestID << " " << sr->machineID << " " << sr->mailbox << " " << sr->arg1 << " " << sr->arg2 << " " << sr->arg3;
			}

			sendReply(outPktHdr, outMailHdr, ss);
		}
	}
}

void Server() {

	//Initialization
	vector<ServerLock*>* serverLocks = new vector<ServerLock*>;
	vector<ServerCV*>* serverCVs = new vector<ServerCV*>;
	vector<ServerMV*>* serverMVs = new vector<ServerMV*>;
	vector<ServerRequest*>* serverRQs = new vector<ServerRequest*>;

	cout << "Running server\n";
	cout << "Set to handle "<<NUM_SERVERS<<" servers.\n";
			
	//uniqueID generator for shared variables
	int uniqueID = myMachineID * 100;

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
		outMailHdr->from = myMachineID;

		//Pull out message type
		int type;
		stringstream ss; //new one every time to be safe
		ss<<buffer;
		ss>>type;

		//Decide what to do based on message type
		string name;
		int lockNum, cvNum, mvSiz, mvNum, mvPos, mvVal;
		stringstream replyStream;
		//First if statement does server vs. client request code
		if(type%100==type) {

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
						NewServerRequest(serverRQs, name, SC_Server_CreateLock, inPktHdr->from, inMailHdr->from, 0, 0, 0);
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

					if(lockNum / 100 != myMachineID) {
						NewServerRequest(serverRQs, NULL, SC_Server_DestroyLock, inPktHdr->from, inMailHdr->from, lockNum, 0, 0);
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
						NewServerRequest(serverRQs, name, SC_Server_CreateCondition, inPktHdr->from, inMailHdr->from, 0, 0, 0);
						/*
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
						*/
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

					if(cvNum / 100 != myMachineID) {
						NewServerRequest(serverRQs, NULL, SC_Server_DestroyCondition, inPktHdr->from, inMailHdr->from, cvNum, 0, 0);
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
					DEBUG('T', "Acquire lock %s for machine %d, mailbox %d\n", serverLocks->at(lockNum)->name.c_str(),
						  inPktHdr->from, inMailHdr->from);

					if(lockNum / 100 != myMachineID) {
						NewServerRequest(serverRQs, NULL, SC_Server_Acquire, inPktHdr->from, inMailHdr->from, lockNum, 0, 0);
					} else {
						lockNum = lockNum % 100;
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
					}
					break;
				}
				case SC_Release: {
					DEBUG('S', "Message: Release\n");
					ss >> lockNum; //get lock ID
					DEBUG('T', "Release lock %s for machine %d, mailbox %d\n", serverLocks->at(lockNum)->name.c_str(),
						  inPktHdr->from, inMailHdr->from);

					if(lockNum / 100 != myMachineID) {
						NewServerRequest(serverRQs, NULL, SC_Server_Acquire, inPktHdr->from, inMailHdr->from, lockNum, 0, 0);
					} else {
						lockNum = lockNum % 100;
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
					}
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
								   (serverCVs->at(cvNum)->lockID != lockNum && serverCVs->at(cvNum)->lockID != -1)) {
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
								   (serverCVs->at(cvNum)->lockID != lockNum && serverCVs->at(cvNum)->lockID != -1)) {
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
						NewServerRequest(serverRQs, name, SC_Server_CreateMV, inPktHdr->from, inMailHdr->from, 0, 0, 0);
						/*
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
						*/
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

					if(mvNum / 100 != myMachineID) {
						NewServerRequest(serverRQs, NULL, SC_Server_DestroyMV, inPktHdr->from, inMailHdr->from, mvNum, 0, 0);
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
					DEBUG('T', "Set MV %s at postition %d to %d for machine %d, mailbox %d\n",
						  serverMVs->at(mvNum)->name.c_str(),
						  mvPos, mvVal, inPktHdr->from, inMailHdr->from);

					if(mvNum / 100 != myMachineID) {
						NewServerRequest(serverRQs, NULL, SC_Server_SetMV, inPktHdr->from, inMailHdr->from, mvNum, mvPos, mvVal);
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
					DEBUG('T', "Get MV %s at postition %d for machine %d, mailbox %d\n",
						  serverMVs->at(mvNum)->name.c_str(),
						  mvPos, inPktHdr->from, inMailHdr->from);

					if(mvNum / 100 != myMachineID) {
						NewServerRequest(serverRQs, NULL, SC_Server_GetMV, inPktHdr->from, inMailHdr->from, mvNum, mvPos, 0);
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
		else if(type / 100 == 1) { //Handle a server request
			//Variables used to hold server request data for processing
			int requestID;
			int machineID;
			int mailbox;
			int arg1;
			int arg2;
			int arg3;

			//pull some info right away since these are required by all requests
			ss >> requestID;
			ss >> machineID;
			ss >> mailbox;

			switch (type) {
				case SC_Server_CreateLock: {
					DEBUG('S', "Message: Server Create lock\n");
					ss >> name; //pull the lock name					
					DEBUG('T', "SR from %d: Create lock request %s for client %d, mailbox %d\n", inPktHdr->from, name.c_str(), machineID,
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
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 0);
					}
					else { //Lock does exist, so just give its ID to the client and reply with yes to server making request
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 1);						
						sendReplyToClient(machineID, mailbox, existingLockID + uniqueID);
					}
					break;
				}
				case SC_Server_DestroyLock: {
					DEBUG('S', "Message: Destroy lock\n");
					ss >> lockNum; //get lock ID
					DEBUG('T', "SR from %d: Set destroy lock %d for machine %d, mailbox %d\n", inPktHdr->from, lockNum, machineID, mailbox);

					//If it's not in our indexes, we don't have it, so reply no
					if(lockNum / 100 != myMachineID) {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 0);
					} else {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 1);							
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
					ss >> name; //pull the condition name					
					DEBUG('T', "SR from %d: Create condition request %s for client %d, mailbox %d\n", inPktHdr->from, name.c_str(), machineID,
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
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 0);
					}
					else { //Condition does exist, so just give its ID to the client and reply with yes to server making request
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 1);						
						sendReplyToClient(machineID, mailbox, existingCVID + uniqueID);
					}
					break;
				}
				case SC_Server_DestroyCondition: {
					DEBUG('S', "Message: Destroy condition\n");
					ss >> cvNum; //get CV ID
					DEBUG('T', "SR from %d: Set destroy condition %d for machine %d, mailbox %d\n", inPktHdr->from, lockNum, machineID, mailbox);

					//If it's not in our indexes, we don't have it, so reply no
					if(cvNum / 100 != myMachineID) {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 0);
					} else {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 1);
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
					if(lockNum / 100 != myMachineID) {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 0);
					} else {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 1);						
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
								PacketHeader* temp_outPktHdr = new PacketHeader(); 
								MailHeader* temp_outMailHdr = new MailHeader();
			
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
					DEBUG('T', "SR from %d: Release lock %d for machine %d, mailbox %d\n", inPktHdr->from, lockNum, machineID, mailbox);

					//If it's not in our indexes, we don't have it, so reply no
					if(lockNum / 100 != myMachineID) {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 0);
					} else {
						lockNum = lockNum % 100;							
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 1);

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
				case SC_Server_Signal: {
					DEBUG('S', "Message: Signal\n");
					ss >> cvNum >> lockNum; //get lock and CV num
					DEBUG('T', "SR from %d: Signal CV %s for machine %d, mailbox %d\n", serverCVs->at(cvNum)->name.c_str(),
						  inPktHdr->from, inMailHdr->from);

					break;
				}
				case SC_Server_Wait: {
					DEBUG('S', "Message: Wait\n");
					ss >> cvNum >> lockNum; //get lock and CV num
					DEBUG('T', "SR from %d: Wait on CV %s for machine %d, mailbox %d\n", serverCVs->at(cvNum)->name.c_str(),
						  inPktHdr->from, inMailHdr->from);

					break;
				}
				case SC_Server_Broadcast: {
					DEBUG('S', "Message: Broadcast\n");
					ss >> cvNum >> lockNum; //get lock and CV num
					DEBUG('T', "SR from %d: Broadcast CV %s for machine %d, mailbox %d\n", serverCVs->at(cvNum)->name.c_str(),
						  inPktHdr->from, inMailHdr->from);

					break;
				}
				case SC_Server_CreateMV: {
					DEBUG('S', "Message: Server Create monitor variable\n");
					ss >> name; //pull the mv name					
					DEBUG('T', "SR from %d: Create monitor variable request %s for client %d, mailbox %d\n", inPktHdr->from, name.c_str(), machineID,
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
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 0);
					}
					else { //Monitor variable does exist, so just give its ID to the client and reply with yes to server making request
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 1);						
						sendReplyToClient(machineID, mailbox, existingMVID + uniqueID);
					}
					break;
				}
				case SC_Server_DestroyMV: {
					DEBUG('S', "Message: Destroy monitor variable\n");
					ss >> mvNum; //get MV ID
					DEBUG('T', "SR from %d: Set destroy monitor variable %d for machine %d, mailbox %d\n", inPktHdr->from, mvNum, machineID, mailbox);

					//If it's not in our indexes, we don't have it, so reply no
					if(mvNum / 100 != myMachineID) {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 0);
					} else {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 1);						
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
					DEBUG('T', "SR from %d: SetMV %d at postition %d to %d for machine %d, mailbox %d\n", inPktHdr->from, mvNum,
						  mvPos, mvVal, machineID, mailbox);

					//If it's not in our indexes, we don't have it, so reply no
					if(mvNum / 100 != myMachineID) {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 0);
					} else {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 1);						
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
					ss >> mvNum >> mvPos >> mvVal;
					DEBUG('T', "SR from %d: GetMV %d at postition %d to %d for machine %d, mailbox %d\n", inPktHdr->from, mvNum,
						  mvPos, mvVal, machineID, mailbox);

					//If it's not in our indexes, we don't have it, so reply no
					if(mvNum / 100 != myMachineID) {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 0);
					} else {
						sendReplyToServer(outPktHdr, outMailHdr, type, requestID, machineID, mailbox, 1);						
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
			switch(type) {
				case SC_Server_Reply_Acquire: {

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
