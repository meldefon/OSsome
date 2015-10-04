// struct.h 
// Structs used in kernel mode for Nachos are defined here
// Created for assignment two, will be used for system calls by user

#ifndef STRUCTS_H
#define STRUCTS_H

#include "synch.h"

// Struct for the assignment two system calls, we shall call it KernelLock
// It will contain the neccessary variables to implement system call functions
struct KernelLock {

	// Default initialization
	KernelLock() {
		lock = NULL; 
		addrSpace = NULL;
		isToBeDeleted = false;
	}

	Lock *lock;
	AddrSpace *addrSpace;
	bool isToBeDeleted;
};

// Struct for the assignment two system calls, we shall call it KernelCondition
// It will contain the neccessary variables to implement system call functions
struct KernelCondition {

	// Default initialization
	KernelCondition() {
		condition = NULL; 
		addrSpace = NULL;
		isToBeDeleted = false;
	}

	Condition *condition;
	AddrSpace *addrSpace;
	bool isToBeDeleted;
};

#endif

