/* testpart1.c
*	Test suite for Project 2, part 1
*	Testing Syscalls:
*		Acquire, Release
*		Wait, Signal, Broadcast
*		CreateLock, DestroyLock
*		CreateCondition, DestroyCondition
*/

# include "syscall.h"

int lock1, lock2, lock3, lock4, lock5;
int cv1, cv2, cv3, cv4, cv5;
int result;



void LockTest(){
	Write("**Testing CreateLock and DestroyLock**\n\n", 40, ConsoleOutput);

	/*Create lock*/
	Write("Test: Create one lock\n", 22, ConsoleOutput);	
	lock1 = CreateLock();
	Write("Result: Success\n\n", 18, ConsoleOutput);

	/*Destroy lock*/
	Write("Test: Destroy one lock\n", 23, ConsoleOutput);
	DestroyLock(lock1);
	Write("Result: Success\n\n", 18, ConsoleOutput);

	/*Destroy Lock that was already destroyed*/
	Write("Test: Destroy lock again\n", 25, ConsoleOutput);
	result = DestroyLock(lock1);
	Write("Result: ", 8, ConsoleOutput);
	if (result == -1){
		Write("Success", 7, ConsoleOutput); 
	}else if(result == 0){ /*lock destroyed*/
		Write("Fail", 4, ConsoleOutput); 
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Create Multiple locks*/
	Write("Test: Create mulitple locks\n", 28, ConsoleOutput);
	lock1 = CreateLock();
	lock2 = CreateLock();
	lock3 = CreateLock();
	lock4 = CreateLock();
	lock5 = CreateLock();
	Write("Result: Success\n\n", 18, ConsoleOutput);

	/*Destroy multiple locks*/
	Write("Test: Destroy mulitple locks\n", 29, ConsoleOutput);
	result = DestroyLock(lock1);
	result += DestroyLock(lock2);
	result += DestroyLock(lock3);
	result += DestroyLock(lock4);
	result += DestroyLock(lock5);
	if (result == 0){
		Write("Result: Success\n\n", 18, ConsoleOutput);
	}else{
		Write("Result: fail - Unable to destroy mulitple locks\n\n", 40, ConsoleOutput);
	}
	
	/*Destroy lock that doesn't exist - minimum case*/
	Write("Test: Destroy invalid lock - Min case\n", 38, ConsoleOutput);
	result = DestroyLock(-1);
	Write("Result: ", 8, ConsoleOutput);
	if (result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail - Lock deleted", 19, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Destroy lock that doesn't exist - maximum case*/
	Write("Test: Destroy invalid lock - Max case\n", 38, ConsoleOutput);
	result = DestroyLock(9999);
	Write("Result: ", 8, ConsoleOutput);
	if (result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail - Lock deleted", 19, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Destroy lock in use by thread - only current owner can delete lock*/
	
	
	Write("**Lock Tests Finished**\n\n", 25, ConsoleOutput);
}

void ConditionTest(){
	Write("**Testing CreateCondition and DestroyCondition**\n\n", 50, ConsoleOutput);
	
	/*Create one CV*/
	Write("Test: Create CV\n", 16, ConsoleOutput);
	cv1 = CreateCondition();
	Write("Result: Success\n\n", 18, ConsoleOutput);

	/*Destroy one CV*/
	Write("Test: Destroy one CV\n", 21, ConsoleOutput);
	DestroyCondition(cv1);
	Write("Result: Success\n\n", 18, ConsoleOutput);

	/*Destroy CV that was already destroyed*/
	Write("Test: Destroy CV again\n", 23, ConsoleOutput);
	result = DestroyCondition(cv1);
	Write("Result: ", 8, ConsoleOutput);
	if (result == -1){
		Write("Success", 7, ConsoleOutput); 
	}else if(result == 0){ /*cv destroyed*/
		Write("Fail - Condition destroyed", 26, ConsoleOutput); 
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Create Multiple CVs*/
	Write("Test: Create multiple CVs\n", 26, ConsoleOutput);
	cv1 = CreateCondition();
	cv2 = CreateCondition();
	cv3 = CreateCondition();
	cv4 = CreateCondition();
	cv5 = CreateCondition();
	Write("Result: Success\n\n", 18, ConsoleOutput);

	/*Destroy Mulitple CVs*/
	Write("Test: Destroy multiple CVs\n", 27, ConsoleOutput);
	result = DestroyCondition(cv1);
	result += DestroyCondition(cv2);
	result += DestroyCondition(cv3);
	result += DestroyCondition(cv4);
	result += DestroyCondition(cv5);
	if (result == 0){
		Write("Result: Success\n\n", 18, ConsoleOutput);
	}else{
		Write("Result: fail - Unable to destroy mulitple CVs\n\n", 40, ConsoleOutput);
	}

	/*Destroy CV that doesn't exist - min case*/
	Write("Test: Destroy invalid CV - Min case\n", 36, ConsoleOutput);
	result = DestroyCondition(-1);
	Write("Result: ", 8, ConsoleOutput);
	if (result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);
	
	/*Destroy CV that doesn't exist - max case*/
	Write("Test: Destroy invalid CV - Max case\n", 36, ConsoleOutput);
	result = DestroyCondition(9999);
	Write("Result: ", 8, ConsoleOutput);
	if (result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Destroy CV in use*/

	
	Write("**Condition Tests Finished**\n\n", 30, ConsoleOutput);
}

void AcquireAndReleaseTest(){
	Write("**Testing Acquire and Release**\n\n", 33, ConsoleOutput);
	
	/*Acquire a lock*/
	Write("Test: Acquire a lock\n", 21, ConsoleOutput);
	lock1 = CreateLock();
	result = Acquire(lock1);
	Write("Result: ", 8,ConsoleOutput);
	if (result == 0){
		Write("Success", 7,ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);


	/*Release a lock*/
	Write("Test: Release a lock\n", 21, ConsoleOutput);
	result = Release(lock1);
	Write("Result: ", 8,ConsoleOutput);
	if (result == 0){
		Write("Success", 7,ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);
	
	Write("Test cleanup\n", 13, ConsoleOutput);
	DestroyLock(lock1);
	Write("\n\n", 2, ConsoleOutput);

	/*Acquire a deleted lock - user's responsibility
	Write("Test: Acquire a deleted lock\n", 29, ConsoleOutput);
	lock2 = CreateLock();
	DestroyLock(lock2);
	result = Acquire(lock2);
	Write("Result: ", 8,ConsoleOutput);
	if (result == -1){
		Write("Success", 7,ConsoleOutput);
	}else{
		Write("Fail - acquired deleted lock", 28, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);*/

	/*Acquire a lock with invalid index*/
	Write("Test: Acquire invalid lock - Min case\n", 38, ConsoleOutput);
	result = Acquire(-1);
	Write("Result: ", 8, ConsoleOutput);
	if (result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail - acquired invalid lock", 28, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Acquire a lock with invalid index - max case*/
	Write("Test: Acquire invalid lock - Max case\n", 38, ConsoleOutput);
	result = Acquire(9999);
	Write("Result: ", 8, ConsoleOutput);
	if (result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail - acquired invalid lock", 28, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Release a lock with invalid index*/
	Write("Test: Release invalid lock - Min case\n", 38, ConsoleOutput);
	result = Release(-1);
	Write("Result: ", 8, ConsoleOutput);
	if (result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Release a lock with invalid index - max case*/
	Write("Test: Release invalid lock - Max case\n", 38, ConsoleOutput);
	result = Release(9999);
	Write("Result: ", 8, ConsoleOutput);
	if (result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Acquire a lock that is already acquired - user responsibility
	Write("Test: Acquire a lock that is already acquired\n", 46, ConsoleOutput);
	lock2 = CreateLock();
	Acquire(lock2);
	result = Acquire(lock2);
	Write("Result: ", 8,ConsoleOutput);
	if (result == -1){
		Write("Success", 7,ConsoleOutput);
	}else{
		Write("Fail - acquired lock", 20, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);*/

	
	/*Release a deleted lock*/
	/*Release a lock that was not acquired*/

	Write("**Acquire and Release Tests Finished**\n\n", 40, ConsoleOutput);
}

void WaitSignalBroadcastTest(){
	Write("**Testing Wait, Signal and Broadcast**\n\n", 40, ConsoleOutput);

	/*Wait - Valid CV, Invalid Lock - min*/
	Write("Test: Wait - valid CV, invalid Lock - Min case\n", 47, ConsoleOutput);
	lock1 = -1;
	cv1 = CreateCondition();
	Acquire(lock1);
	result = Wait(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Wait - Valid CV, Invalid Lock - max*/
	Write("Test: Wait - valid CV, invalid Lock - Max case\n", 47, ConsoleOutput);
	lock1 = 9999;
	cv1 = CreateCondition();
	Acquire(lock1);
	result = Wait(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Wait - Valid Lock, Invalid CV - min*/
	Write("Test: Wait - valid Lock, invalid CV - Min case\n", 47, ConsoleOutput);
	lock1 = CreateLock();
	cv1 = -1;
	Acquire(lock1);
	result = Wait(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Wait - Valid Lock, Invalid CV - max*/
	Write("Test: Wait - valid Lock, invalid CV - Max case\n", 47, ConsoleOutput);
	lock1 = CreateLock();
	cv1 = 9999;
	Acquire(lock1);
	result = Wait(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Wait - Valid CV and Lock*/


	/*Signal - Valid CV, Invalid Lock - min*/
	Write("Test: Signal - valid CV, invalid Lock - Min case\n", 49, ConsoleOutput);
	lock1 = -1;
	cv1 = CreateCondition();
	Acquire(lock1);
	result = Signal(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Signal - Valid CV, Invalid Lock - max*/
	Write("Test: Signal - valid CV, invalid Lock - Max case\n", 49, ConsoleOutput);
	lock1 = 9999;
	cv1 = CreateCondition();
	Acquire(lock1);
	result = Signal(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Signal - Valid Lock, Invalid CV - min*/
	Write("Test: Signal - valid Lock, invalid CV - Min case\n", 49, ConsoleOutput);
	lock1 = CreateLock();
	cv1 = -1;
	Acquire(lock1);
	result = Signal(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Signal - Valid Lock, Invalid CV - max*/
	Write("Test: Signal - valid Lock, invalid CV - Max case\n", 49, ConsoleOutput);
	lock1 = CreateLock();
	cv1 = 9999;
	Acquire(lock1);
	result = Signal(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Broadcast - Valid CV, Invalid Lock - min*/
	Write("Test: Broadcast - valid CV, invalid Lock - Min case\n", 52, ConsoleOutput);
	lock1 = -1;
	cv1 = CreateCondition();
	Acquire(lock1);
	result = Broadcast(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Broadcast - Valid CV, Invalid Lock - max*/
	Write("Test: Broadcast - valid CV, invalid Lock - Max case\n", 52, ConsoleOutput);
	lock1 = 9999;
	cv1 = CreateCondition();
	Acquire(lock1);
	result = Broadcast(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Broadcast - Valid Lock, Invalid CV - min*/
	Write("Test: Broadcast - valid Lock, invalid CV - Min case\n", 52, ConsoleOutput);
	lock1 = CreateLock();
	cv1 = -1;
	Acquire(lock1);
	result = Broadcast(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Broadcast - Valid Lock, Invalid CV - max*/
	Write("Test: Broadcast - valid Lock, invalid CV - Max case\n", 52, ConsoleOutput);
	lock1 = CreateLock();
	cv1 = 9999;
	Acquire(lock1);
	result = Broadcast(cv1, lock1);
	Write("Result: ", 8, ConsoleOutput);
	if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	Write("**Wait, Signal and Broadcast Tests Finished**\n\n", 47, ConsoleOutput);
}


int main(){

	Write("\nProject 2, Part 1 Tests\n\n", 26, ConsoleOutput);
	
	/*Test CreateLock and DestroyLock*/		
	LockTest();

	/*Test CreateCondition and DestroyCondition*/
	ConditionTest();

	/*Test Acquire and Release*/
	AcquireAndReleaseTest();

	/*Test Wait, Signal, and Broadcast*/
	WaitSignalBroadcastTest();
}