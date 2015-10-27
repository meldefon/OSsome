/* testpart2.c
*	Test suite for Project 2, part 2
*	Testing Fork and Exec
*/
# include "syscall.h"

int result, lock, cv;

/*Thread 1 for ForkTest*/
void ForkTest_t1(){
	Write("Forked thread 1\n", 16, ConsoleOutput);
	Exit(0);
	return;
}
/*Thread 2 for ForkTest*/
void ForkTest_t2(){
	Write("Forked thread 2\n", 16, ConsoleOutput);
	Exit(0);
	return;
}

/*Thread 3 for ForkTest*/
void ForkTest_t3(){
	Write("Forked thread 3\n", 16, ConsoleOutput);
	Exit(0);
	return;
}

/*Thread 4 for ForkTest*/
void ForkTest_t4(){
	Write("Forked thread 4\n\n", 17, ConsoleOutput);
	Exit(0);
	return;
}

/*Test for Exec syscall*/
void ExecTest(){
	Write("The following tests for Exec will run:\n", 39, ConsoleOutput);

	/*Exec a process*/
	Write("Test 1: Exec a process\n", 23, ConsoleOutput);
	Exec("../test/testfiles",17);

	/*Exec another process*/
	Write("Test 2: Exec another process\n", 29, ConsoleOutput);
    Exec("../test/testfiles",17);

	

	/*Exec with invalid input*/
	Write("Test 3: Exec with invalid input\n", 32, ConsoleOutput);
	Exec(1, 1);


	Write("\n\n", 2, ConsoleOutput);
}

/*Test for Fork syscall*/
void ForkTest(){
	Write("The following tests for Fork will run:\n", 39, ConsoleOutput);
	
	/*Fork one thread*/
	Write("Test 1: Fork one thread\n", 24, ConsoleOutput);
	Fork(ForkTest_t1);
	

	/*Fork more threads*/
	Write("Test 2: Fork 3 more threads\n", 29, ConsoleOutput);
	Fork(ForkTest_t2);
	Fork(ForkTest_t3);
	Fork(ForkTest_t4);
	

	/*Fork thread with invalid input*/
	Write("Test 3: Fork thread with invalid input\n", 39, ConsoleOutput);
	Fork(-1);
	Write("\n", 1, ConsoleOutput);
	
}



int main(){
	Write("\nProject 2, Part 2 Tests\n\n", 26, ConsoleOutput);

	/*Test Fork*/
	ForkTest();

	/*Test Exec*/
	ExecTest();

	/*Exec test on Passport Office*/
	Exec("../test/threadtest",18);
    Exec("../test/threadtest",18);

	Exit(0);
}