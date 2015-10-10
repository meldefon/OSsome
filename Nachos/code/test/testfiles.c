/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void doIt(){
  Write("\nDo it called", 36, ConsoleOutput);
  Exit(0);
  return;
}

int main() {
  OpenFileId fd;
  int bytesread;
  char buf[20];

  Fork(doIt);

  Write("\nOperating Systems is an easy class.", 36, ConsoleOutput);
/*
    Create("testfile", 8);
    fd = Open("testfile", 8);

    Write("testing a write\n", 16, fd );
    Close(fd);


    fd = Open("testfile", 8);
    bytesread = Read( buf, 100, fd );
    Write( buf, bytesread, ConsoleOutput );
    Close(fd);*/
}
