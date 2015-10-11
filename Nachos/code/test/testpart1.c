/* testpart1.c
*	Test suite for Project 2, part 1
*/

# include "syscall.h"

int lock1, lock2, lock3, lock4, lock5;
int cv1, cv2, cv3, cv4, cv5;
int result;



void LockTest(){
	Write("Testing CreateLock and DestroyLock\n\n", 36, ConsoleOutput);

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
	if (result == -1){ /*returned bad id*/
		Write("-1", 1, ConsoleOutput); 
	}else if(result == 0){ /*lock destroyed*/
		Write("0", 1, ConsoleOutput); 
	}else{
		Write("?", 1, ConsoleOutput); 
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

	/*Destroy lock that doesn't exist*/
	Write("Test: Destroy invalid lock\n", 27, ConsoleOutput);
	result = DestroyLock(-1);
	Write("Result: ", 8, ConsoleOutput);
	if (result == -1){
		Write("Success", 22, ConsoleOutput);
	}else if(result == 0){
		Write("Fail - Lock deleted", 20, ConsoleOutput);
	}else{
		Write("?", 1, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);
	
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
	
	/*Destroy lock in use by thread*/
	Write("Test: Destroy lock used by thread\n", 23, ConsoleOutput);
	
	
	Write("**Lock Tests Finished**\n\n", 25, ConsoleOutput);
}

void ConditionTest(){
	Write("Testing CreateCondition and DestroyCondition\n\n", 46, ConsoleOutput);
	
	/*Create one CV*/
	Write("Test: Create CV\n", 16, ConsoleOutput);
	cv1 = CreateCondition();
	Write("Result: Success\n\n", 18, ConsoleOutput);

	/*Destroy one CV*/
	Write("Test: Destroy one CV\n", 21, ConsoleOutput);
	DestroyCondition(cv1);
	Write("Result: Success\n\n", 18, ConsoleOutput);

	/*Destroy CV that was already destroyed*/

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


	/*Destroy CV that doesn't exist*/
	Write("Test: Destroy CV that doesn't exist\n", 36, ConsoleOutput);
	
	/*Destroy CV in use*/
	Write("Test: Destroy a CV in use\n", 26, ConsoleOutput);
	


	Write("**Condition Tests Finished**\n\n", 30, ConsoleOutput);
}

void AcquireAndReleaseTest(){
	/*Acquire a lock*/
	Write("Test: Acquire a lock\n", 21, ConsoleOutput);
	lock1 = CreateLock();
	result = Acquire(lock1);
	Write("Result: ", 8,ConsoleOutput);
	if (result == 0){
		Write("Success", 7,ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);

	/*Acquire a lock with invalid index*/
	/*Acquire a deleted lock*/
	/*Acquire a lock that is already acquired*/


	/*Release a lock*/
	/*Release a lock with invalid index*/
	/*Release a deleted lock*/
	/*Release a lock that was not acquired*/

	Write("**Acquire and Release Tests Finished**\n\n", 40, ConsoleOutput);
}


int main(){

	Write("\nProject 2, Part 1 Tests\n\n", 26, ConsoleOutput);

	/*Test CreateCondition and DestroyCondition*/
	ConditionTest();
	
	/*Test CreateLock and DestroyLock*/		
	LockTest();

	/*Test Acquire and Release*/
	AcquireAndReleaseTest();

	/*Test Wait, Signal, and Broadcast*/

}