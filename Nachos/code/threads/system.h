// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include <vector>
#include "structs.h"
#include "bitmap.h"
#include "synch.h"

//#include "table.h"
//#include "addrspace.h"
using namespace std;

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

//class BitMap;
extern BitMap* freePageBitMap;

// New globals implemented for assignment 2, holds user locks and condtions
extern vector<KernelLock*> locks;
extern vector<KernelCondition*> conditions;

// IPT table for assignment 3
extern IPTEntry *IPT;

//Global process table
extern vector<ProcessStruct*>* processTable;
extern Lock* progLock;

// Some new global locks for syscalls
extern Lock sysLock;
extern Lock sysCondition;

//Keeps track of next TLB entry to write to
extern int currentTLB;

//Swapfile stuff
extern OpenFile* swapFile;
extern BitMap* swapFileBitMap;
extern int swapFileSize;

//Another copy of the machineID to be used by syscall code
extern int machineIDCopy;


#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
