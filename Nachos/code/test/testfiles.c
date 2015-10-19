/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void doIt(){
  Write("doIt called\n", 12, ConsoleOutput);
  Exit(0);
  return;
}

int main() {
    OpenFileId fd, fd2;
    int bytesread;
    char buf[20];



    /*fd = Open("../test/matmult",15);*/
    /*fd2 = Open("../test/sort",12);*/
    Exec("../test/sort",12);
    /*Exec(fd);
    Exec(fd);
    Exec(fd2);*/


    Fork(doIt);
    /*Fork(doIt);*/

}
