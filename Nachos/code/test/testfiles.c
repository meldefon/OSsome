/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void doIt(){
  Write("doIt called\n", 13, ConsoleOutput);
  Exit(0);
  return;
}

int main() {
    OpenFileId fd;
    int bytesread;
    char buf[20];



    fd = Open("matmult",9);
    Exec(fd);


    Fork(doIt);

}
