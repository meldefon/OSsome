//
// Created by meldefon on 11/3/15.
//

#ifndef OSSOME_SERVERSTRUCTS_H
#define OSSOME_SERVERSTRUCTS_H

#include "../machine/network.h"
#include <string>
#include <queue>

enum ServerLockState {Busy, Available};

struct ServerLock{

    string name;
    ServerLockState state;
    int ownerMachineID;
    int ownerMailboxNum;
    queue<PacketHeader*>* packetWaitQ;
    queue<MailHeader*>* mailWaitQ;
    bool isToBeDeleted;

};


struct ServerCV{

    string name;
    queue<PacketHeader*>* packetWaitQ;
    queue<MailHeader*>* mailWaitQ;    
    int lockID;
    bool isToBeDeleted;

};

struct ServerMV{

    string name;
    int* vals;
    int length;
    bool isToBeDeleted;

};

struct ServerRequest{

    string name; //potential name of a Lock, CV, or MV
    int requestID; //index in request vector
    int requestType; //type of case
    int machineID; //machine ID of client
    int mailbox; //mailbox of client
    int noCount; //number of no's received
    int arg1;
    int arg2;
    int arg3;
    bool yes; //if yes has been received
};

#endif //OSSOME_SERVERSTRUCTS_H
