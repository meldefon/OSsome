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
    queue<PacketHeader*>* waitQ;

};


struct ServerCV{

    queue<PacketHeader*>* waitQ;
    int lockID;

};

struct ServerMV{

    int* vals;
    int length;

};


#endif //OSSOME_SERVERSTRUCTS_H
