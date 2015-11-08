# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = ../gnu/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort lockTests CVTests serverCode testfiles testpart1 testpart2 threadtest

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

lockTests.o: lockTests.c
	$(CC) $(CFLAGS) -c lockTests.c
lockTests: lockTests.o start.o
	$(LD) $(LDFLAGS) start.o lockTests.o -o lockTests.coff
	../bin/coff2noff lockTests.coff lockTests

CVTests.o: CVTests.c
	$(CC) $(CFLAGS) -c CVTests.c
CVTests: CVTests.o start.o
	$(LD) $(LDFLAGS) start.o CVTests.o -o CVTests.coff
	../bin/coff2noff CVTests.coff CVTests

serverCode.o: serverCode.c
	$(CC) $(CFLAGS) -c serverCode.c
serverCode: serverCode.o start.o
	$(LD) $(LDFLAGS) start.o serverCode.o -o serverCode.coff
	../bin/coff2noff serverCode.coff serverCode

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

testpart1.o: testpart1.c
	$(CC) $(CFLAGS) -c testpart1.c
testpart1: testpart1.o start.o
	$(LD) $(LDFLAGS) start.o testpart1.o -o testpart1.coff
	../bin/coff2noff testpart1.coff testpart1

testpart2.o: testpart2.c
	$(CC) $(CFLAGS) -c testpart2.c
testpart2: testpart2.o start.o
	$(LD) $(LDFLAGS) start.o testpart2.o -o testpart2.coff
	../bin/coff2noff testpart2.coff testpart2

threadtest.o: threadtest.c
	$(CC) $(CFLAGS) -c threadtest.c
threadtest: threadtest.o start.o
	$(LD) $(LDFLAGS) start.o threadtest.o -o threadtest.coff
	../bin/coff2noff threadtest.coff threadtest



clean:
	rm -f *.o *.coff testpart1 testpart2 threadtest 