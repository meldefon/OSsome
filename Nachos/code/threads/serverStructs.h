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

    /*This is the data that will get sent to other servers*/
    string name; //potential name of a Lock, CV, or MV
    int requestID; //index in request vector
    int requestType; //type of case
    int machineID; //machine ID of client
    int mailbox; //mailbox of client
    int arg1;
    int arg2;
    int arg3;
    
    /* This is the data that will only be used server side and
       not sent to other servers */
    bool yes; //if yes has been received
    int noCount; //number of no's received
    bool lockFound;
    bool cvFound;
    int lockCount;
    int cvCount;
    PacketHeader* replyServerMachineID;
    MailHeader* replyServerMailbox;
};

#endif //OSSOME_SERVERSTRUCTS_H
