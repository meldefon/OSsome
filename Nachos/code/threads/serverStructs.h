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
    int ownerMalboxNum;
    queue<PacketHeader*>* packetWaitQ;
    queue<MailHeader*>* mailWaitQ;
    bool isToBeDeleted;

};


struct ServerCV{

    queue<PacketHeader*>* packetWaitQ;
    queue<MailHeader*>* mailWaitQ;    
    int lockID;
    bool isToBeDeleted;

};

struct ServerMV{

    int* vals;
    int length;
    bool isToBeDeleted;

};


#endif //OSSOME_SERVERSTRUCTS_H
