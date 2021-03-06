/* syscalls.h
 * 	Nachos system call interface.  These are Nachos kernel operations
 * 	that can be invoked from user programs, by trapping to the kernel
 *	via the "syscall" instruction.
 *
 *	This file is included by user programs and by the Nachos kernel. 
 *
 * Copyright (c) 1992-1993 The Regents of the University of California.
 * All rights reserved.  See copyright.h for copyright notice and limitation 
 * of liability and disclaimer of warranty provisions.
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "copyright.h"

/* system call codes -- used by the stubs to tell the kernel which system call
 * is being asked for
 */
#define SC_Halt		0
#define SC_Exit		1
#define SC_Exec		2
#define SC_Join		3
#define SC_Create	4
#define SC_Open		5
#define SC_Read		6
#define SC_Write	7
#define SC_Close	8
#define SC_Fork		9
#define SC_Yield	10

/* Assignment 3 systems call that are going to be implemented
* we will keep them separate since we do not want to mix up
* old code with new code for debugging purposes
*/
#define SC_Acquire			11
#define SC_Release			12
#define SC_Wait				13
#define SC_Signal			14
#define	SC_Broadcast		15
#define SC_CreateLock		16
#define SC_DestroyLock		17
#define SC_CreateCondition	18
#define SC_DestroyCondition	19
#define SC_Rand             20
#define SC_Printf           21
#define SC_Scanf			22
#define SC_CreateMV			23
#define SC_DestroyMV        24
#define SC_SetMV			25
#define SC_GetMV			26

#define SC_Server_Acquire           111
#define SC_Server_Release		    112
#define SC_Server_Wait1			    113
#define SC_Server_Signal1			114
#define	SC_Server_Broadcast1		115
#define SC_Server_CreateLock		116
#define SC_Server_DestroyLock		117
#define SC_Server_CreateCondition	118
#define SC_Server_DestroyCondition	119
#define SC_Server_Rand              120
#define SC_Server_Printf            121
#define SC_Server_Scanf			    122
#define SC_Server_CreateMV			123
#define SC_Server_DestroyMV         124
#define SC_Server_SetMV			    125
#define SC_Server_GetMV			    126
#define SC_Server_Signal2			127
#define SC_Server_Signal3			128
#define SC_Server_Wait2				129
#define SC_Server_Wait3				130
#define SC_Server_Broadcast2		131
#define SC_Server_Broadcast3		132
#define SC_Server_Broadcast4		133

#define SC_ServerReply_Acquire           	211
#define SC_ServerReply_Release		    	212
#define SC_ServerReply_Wait1			    213
#define SC_ServerReply_Signal1				214
#define	SC_ServerReply_Broadcast1		    215
#define SC_ServerReply_CreateLock			216
#define SC_ServerReply_DestroyLock			217
#define SC_ServerReply_CreateCondition		218
#define SC_ServerReply_DestroyCondition		219
#define SC_ServerReply_Rand              	220
#define SC_ServerReply_Printf            	221
#define SC_ServerReply_Scanf			    222
#define SC_ServerReply_CreateMV				223
#define SC_ServerReply_DestroyMV         	224
#define SC_ServerReply_SetMV			    225
#define SC_ServerReply_GetMV			    226
#define SC_ServerReply_Signal2				227
#define SC_ServerReply_Signal3				228
#define SC_ServerReply_Wait2				229
#define SC_ServerReply_Wait3				230
#define SC_ServerReply_Broadcast2			231
#define SC_ServerReply_Broadcast3			232
#define SC_ServerReply_Signal4				233
#define SC_ServerReply_Wait4				234
#define SC_ServerReply_Wait5				235
#define SC_ServerReply_Broadcast4			236


#define MAXFILENAME 256

/*Give the server a special address*/
#define SERVER_M 0
/*#define NUM_SERVERS 2*/

#ifndef IN_ASM



/* A unique identifier for an open Nachos file. */
typedef int OpenFileId;

/* The system call interface.  These are the operations the Nachos
 * kernel needs to support, to be able to run user programs.
 *
 * Each of these is invoked by a user program by simply calling the 
 * procedure; an assembly language stub stuffs the system call code
 * into a register, and traps to the kernel.  The kernel procedures
 * are then invoked in the Nachos kernel, after appropriate error checking, 
 * from the system call entry point in exception.cc.
 */

/* Stop Nachos, and print out performance stats */
void Halt();
 

/* Address space control operations: Exit, Exec, and Join */

/* This user program is done (status = 0 means exited normally). */
void Exit(int status);	

/* A unique identifier for an executing user program (address space) */
typedef int SpaceId;	
 
/* Run the executable, stored in the Nachos file "name", and return the 
 * address space identifier
 */
SpaceId Exec(char* name, int size);
 
/* Only return once the the user program "id" has finished.  
 * Return the exit status.
 */
int Join(SpaceId id); 	
 

/* File system operations: Create, Open, Read, Write, Close
 * These functions are patterned after UNIX -- files represent
 * both files *and* hardware I/O devices.
 *
 * If this assignment is done before doing the file system assignment,
 * note that the Nachos file system has a stub implementation, which
 * will work for the purposes of testing out these routines.
 */
 


/* when an address space starts up, it has two open files, representing 
 * keyboard input and display output (in UNIX terms, stdin and stdout).
 * Read and Write can be used directly on these, without first opening
 * the console device.
 */

#define ConsoleInput	0  
#define ConsoleOutput	1  
 
/* Create a Nachos file, with "name" */
void Create(char *name, int size);

/* Open the Nachos file "name", and return an "OpenFileId" that can 
 * be used to read and write to the file.
 */
OpenFileId Open(char *name, int size);

/* Write "size" bytes from "buffer" to the open file. */
void Write(char *buffer, int size, OpenFileId id);

/* Read "size" bytes from the open file into "buffer".  
 * Return the number of bytes actually read -- if the open file isn't
 * long enough, or if it is an I/O device, and there aren't enough 
 * characters to read, return whatever is available (for I/O devices, 
 * you should always wait until you can return at least one character).
 */
int Read(char *buffer, int size, OpenFileId id);

/* Close the file, we're done reading and writing to it. */
void Close(OpenFileId id);



/* User-level thread operations: Fork and Yield.  To allow multiple
 * threads to run within a user program. 
 */

/* Fork a thread to run a procedure ("func") in the *same* address space 
 * as the current thread.
 */
void Fork(int forkArg);

/* Yield the CPU to another runnable thread, whether in this address space 
 * or not. 
 */
void Yield();

/* User-level Lock operations: CreateLock, DestroyLock, Acquire, Release.
 * Allows threads to run within a user program while using synchronization.   
 */

/* Creates a lock for the user program */
int CreateLock(char* name,int len);

 /* Destroys a lock for the user program */
 int DestroyLock(int id);

 /* Acquires a lock for the user program */
 int Acquire(int id);

 /* Release a lock for the user program */	
 int Release(int id);

 /* User-level Condition operations: CreateCondition, DestroyCondition, Signal, Wait, Broadcast.
 * Allows threads to run within a user program while using condition variables.   
 */

 /* Creates a condition variable for the user program */	
int CreateCondition(char* name, int len);

 /* Destroys a condition variable for the user program */
 int DestroyCondition(int id);

 /* Signals a thread for the user program */	
 int Signal(int c, int l);

 /* Allows a thread to wait in the user program */
 int Wait(int c, int l);

 /* Broadcasts to threads for the user program */	
 int Broadcast(int c, int l);

 /* User-level miscellaneous operations: Rand, Printf, Scanf.
 * Allows user to use practical functions
 */

 /* Allows user to generate random number */
 int Rand(int range, int offset);

 /* Allows user to take in integer input */
 int Scanf(); 
 
 /* Allows user to print with integers */
 void Printf(char* string, int length, int Num_1, int Num_2);


/* Creates a MV for the user program */
int CreateMV(char* name,int nameLen, int size);

/* Destroys a MV for the user program */
int DestroyMV(int id);

/* Sets a MV at position index to value value for the user program */
int SetMV(int MVid, int index, int value);

/* Gets a MV at position index for the user program */
int GetMV(int MVid, int index);


#endif /* IN_ASM */

#endif /* SYSCALL_H */
