/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void doIt(){
  Write("Do it called\n", 13, ConsoleOutput);
  Exit(0);
  return;
}

int main() {
    OpenFileId fd;
    int bytesread;
    char buf[20];


    Fork(doIt);
    Fork(doIt);
    Fork(doIt);
    Fork(doIt);
    Fork(doIt);
    Fork(doIt);
    Fork(doIt);
    Fork(doIt);
    Fork(doIt);
    Fork(doIt);



}
