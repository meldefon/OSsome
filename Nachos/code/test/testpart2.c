/* testpart2.c
*	Test suite for Project 2, part 2
*	Testing Fork and Exec
*/
# include "syscall.h"

int result, lock, cv;

/*Thread 1 for ForkTest*/
void ForkTest_t1(){
	Write("Forked thread 1\n\n", 17, ConsoleOutput);
	Exit(0);
}
/*Thread 2 for ForkTest*/
void ForkTest_t2(){
	Write("Forked thread 2\n\n", 17, ConsoleOutput);
	Exit(0);
}

void ExecTest(){
	Write("**Testing Exec**\n\n", 18, ConsoleOutput);
	
	/*Exec a process*/

	/*Exec another process*/

	/*Exec with invalid input - min case*/
	Write("Test: Exec with invalid input - min case\n\n", 42, ConsoleOutput);
	/*result = Exec(-1);*/
	Write("Result: ", 8, ConsoleOutput);
	/*if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}*/
	Write("\n\n", 2, ConsoleOutput);
	
	/*Exec with invalid input - min case*/
	Write("Test: Exec with invalid input - min case\n\n", 42, ConsoleOutput);
	/*result = Exec(9999);*/
	Write("Result: ", 8, ConsoleOutput);
	/*if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}*/
	Write("\n\n", 2, ConsoleOutput);

	Write("**Exec Tests Finished**\n\n", 25, ConsoleOutput);
}

void ForkTest(){
	Write("**Testing Fork**\n\n", 18, ConsoleOutput);
	
	/*Fork one thread*/
	Write("Test: Fork one thread\n\n", 23, ConsoleOutput);
	Fork(ForkTest_t1);
	Write("Result: ", 8, ConsoleOutput);

	Write("\n\n", 2, ConsoleOutput);

	/*Fork more threads*/
	Write("Test: Fork another thread\n\n", 27, ConsoleOutput);
	Fork(ForkTest_t2);
	Write("Result: ", 8, ConsoleOutput);

	Write("\n\n", 2, ConsoleOutput);

	/*Fork thread with invalid input - min case*/
	Write("Test: Fork thread, invalid input - min case\n\n", 45, ConsoleOutput);
	/*result = Fork(-1);*/
	Write("Result: ", 8, ConsoleOutput);
	/*if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}*/
	Write("\n\n", 2, ConsoleOutput);
	
	/*Fork thread with invalid input - min case*/
	Write("Test: Fork thread, invalid input - min case\n\n", 45, ConsoleOutput);
	/*result = Fork(9999);*/
	Write("Result: ", 8, ConsoleOutput);
	/*if(result == -1){
		Write("Success", 7, ConsoleOutput);
	}else{
		Write("Fail", 4, ConsoleOutput);
	}*/
	Write("\n\n", 2, ConsoleOutput);

	Write("**Fork Tests Finished**\n\n", 25, ConsoleOutput);
}



int main(){
	Write("\nProject 2, Part 2 Tests\n\n", 26, ConsoleOutput);

	/*Test Fork*/
	ForkTest();

	/*Test Exec*/
	ExecTest();

	Write("\nEnd of Project 2, Part 2 Tests\n\n", 33, ConsoleOutput);

	Exit(0);
}