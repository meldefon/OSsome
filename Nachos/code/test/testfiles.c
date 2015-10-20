/* testfiles.c
 *	Simple program to test exec system calls
 */

#include "syscall.h"

/*Thread 1 for exec test*/
void testfiles_t1(){
  Write("Forked thread 1 in testfiles\n", 29, ConsoleOutput);
  Exit(0);
  return;
}

/*Thread 2 for exec test*/
void testfiles_t2(){
  Write("Forked thread 2 in testfiles\n", 29, ConsoleOutput);
  Exit(0);
  return;
}

int main() {
    /*OpenFileId fd, fd2;
    int bytesread;
    char buf[20];*/



    /*fd = Open("../test/matmult",15);*/
    /*fd2 = Open("../test/sort",12);*/
    /*Exec("../test/sort",12);*
    Exec("../test/threadtest",18);
    Exec("../test/threadtest",18);
    /*Exec(fd);
    Exec(fd);
    Exec(fd2);*/

    /* Simple program to test exec syscall
     * This user program forks 2 threads */
    Write("Exec user program testfiles\n", 28, ConsoleOutput);

    Fork(testfiles_t1);
    Fork(testfiles_t2);
}
