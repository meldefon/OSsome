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

all: halt matmult matmultX2 sort lockTests CVTests CVTestsWaiter CVTestsSignaller CVTestsBroadcaster MVTests serverCode testfiles testpart1 testpart2 threadtest appClerk cashier clerks customer manager passPClerk picClerk setup

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

matmultX2.o: matmultX2.c
	$(CC) $(CFLAGS) -c matmultX2.c
matmultX2: matmultX2.o start.o
	$(LD) $(LDFLAGS) start.o matmultX2.o -o matmultX2.coff
	../bin/coff2noff matmultX2.coff matmultX2

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

CVTestsWaiter.o: CVTestsWaiter.c
	$(CC) $(CFLAGS) -c CVTestsWaiter.c
CVTestsWaiter: CVTestsWaiter.o start.o
	$(LD) $(LDFLAGS) start.o CVTestsWaiter.o -o CVTestsWaiter.coff
	../bin/coff2noff CVTestsWaiter.coff CVTestsWaiter

CVTestsSignaller.o: CVTestsSignaller.c
	$(CC) $(CFLAGS) -c CVTestsSignaller.c
CVTestsSignaller: CVTestsSignaller.o start.o
	$(LD) $(LDFLAGS) start.o CVTestsSignaller.o -o CVTestsSignaller.coff
	../bin/coff2noff CVTestsSignaller.coff CVTestsSignaller

CVTestsBroadcaster.o: CVTestsBroadcaster.c
	$(CC) $(CFLAGS) -c CVTestsBroadcaster.c
CVTestsBroadcaster: CVTestsBroadcaster.o start.o
	$(LD) $(LDFLAGS) start.o CVTestsBroadcaster.o -o CVTestsBroadcaster.coff
	../bin/coff2noff CVTestsBroadcaster.coff CVTestsBroadcaster

MVTests.o: MVTests.c
	$(CC) $(CFLAGS) -c MVTests.c
MVTests: MVTests.o start.o
	$(LD) $(LDFLAGS) start.o MVTests.o -o MVTests.coff
	../bin/coff2noff MVTests.coff MVTests

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

setup.o: setup.c
	$(CC) $(CFLAGS) -c setup.c
setup: setup.o start.o
	$(LD) $(LDFLAGS) start.o setup.o -o setup.coff
	../bin/coff2noff setup.coff setup

appClerk.o: appClerk.c
	$(CC) $(CFLAGS) -c appClerk.c
appClerk: appClerk.o start.o
	$(LD) $(LDFLAGS) start.o appClerk.o -o appClerk.coff
	../bin/coff2noff appClerk.coff appClerk

cashier.o: cashier.c
	$(CC) $(CFLAGS) -c cashier.c
cashier: cashier.o start.o
	$(LD) $(LDFLAGS) start.o cashier.o -o cashier.coff
	../bin/coff2noff cashier.coff cashier

customer.o: customer.c
	$(CC) $(CFLAGS) -c customer.c
customer: customer.o start.o
	$(LD) $(LDFLAGS) start.o customer.o -o customer.coff
	../bin/coff2noff customer.coff customer

clerks.o: clerks.c
	$(CC) $(CFLAGS) -c clerks.c
clerks: clerks.o start.o
	$(LD) $(LDFLAGS) start.o clerks.o -o clerks.coff
	../bin/coff2noff clerks.coff clerks

manager.o: manager.c
	$(CC) $(CFLAGS) -c manager.c
manager: manager.o start.o
	$(LD) $(LDFLAGS) start.o manager.o -o manager.coff
	../bin/coff2noff manager.coff manager

passPClerk.o: passPClerk.c
	$(CC) $(CFLAGS) -c passPClerk.c
passPClerk: passPClerk.o start.o
	$(LD) $(LDFLAGS) start.o passPClerk.o -o passPClerk.coff
	../bin/coff2noff passPClerk.coff passPClerk

picClerk.o: picClerk.c
	$(CC) $(CFLAGS) -c picClerk.c
picClerk: picClerk.o start.o
	$(LD) $(LDFLAGS) start.o picClerk.o -o picClerk.coff
	../bin/coff2noff picClerk.coff picClerk

clean:
	rm -f *.o *.coff halt matmult matmultX2 sort lockTests CVTests CVTestsWaiter CVTestsSignaller CVTestsBroadcaster MVTests serverCode testfiles testpart1 testpart2 threadtest appClerk cashier clerks customer manager passPClerk picClerk setup
	 