//
// Created by meldefon on 11/3/15.
//

#ifndef OSSOME_SERVERSTRUCTS_H
#define OSSOME_SERVERSTRUCTS_H

#include "../machine/network.h"

enum serverLockState {Busy, Available};

struct serverLock{

    serverLockState state;
    int ownerMachineID;
    int ownerMalboxNum;
    PacketHeader* waitQ;

};


struct serverCV{

    PacketHeader* waitQ;
    int lockID;

};

struct serverMV{

    int* vals;
    int length;

};


#endif //OSSOME_SERVERSTRUCTS_H
