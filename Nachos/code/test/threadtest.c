#include "monitor.h"
#include "setup.h"
#define NULL 0

void Uprintf(char *string, int length, int num_1, int num_2, int num_3, int num_4) {
    Printf(string, length, (num_1 * 100000) + num_2, (num_3 * 100000) + num_4);
}

int main() {

  	int size; /*will be used to take in user input for the sizes of specific variables*/
  	int senatorSize;
	int i;
    int j;
    int k;
    int l;
    int m;
    int n;
	int numberOfSenators;
	int testSuite;
	int numAppClerks;
	int numPicClerks;
	int numCashiers;
	int numPassportClerks;

	Uprintf("For TestSuite, enter 1\nFor Simulation, enter 2", 46, 0, 0, 0, 0);
	testSuite = Scanf();

	if(testSuite == 1)
		testSuite = 0; /*TestSuite();*/
	else {

		Uprintf("Number of ApplicationClerks = ", 30, 0, 0, 0, 0);
		size = Scanf();
		numAppClerks = size;

		initialize(&appClerk, 0, size);

		Uprintf("Number of PictureClerks = ", 26, 0, 0, 0, 0);
		size = Scanf();
		numPicClerks = size;

		initialize(&picClerk, 1, size);

		Uprintf("Number of PassportClerks = ", 27, 0, 0, 0, 0);
		size = Scanf();
		numPassportClerks = size;

		initialize(&passPClerk, 2, size);
		
		Uprintf("Number of Cashiers = ", 21, 0, 0, 0, 0);
		size = Scanf();
		numCashiers = size;

		initialize(&cashier, 3, size);
		
		Uprintf("Number of Customers = ", 22, 0, 0, 0, 0);
		size = Scanf();

		Uprintf("Number of Senators = ", 21, 0, 0, 0, 0);
		numberOfSenators = Scanf();
		
		size += numberOfSenators;

		/* Create global server MVs and initialize everything */
		createServerMVs(size, numberOfSenators);

		/*initialize all the programs here*/ 
		for(j = 0; j < size; j++) {
    		Exec("../test/customer",16);
		}

		for(k = 0; k < numAppClerks; k++) {
    		Exec("../test/appClerk",16);
		}

		for(l = 0; l < numPicClerks; l++) {
    		Exec("../test/picClerk",16);
		}

		for(m = 0; m < numPassportClerks; m++) {
    		Exec("../test/passPClerk",18);
		}

		for(n = 0; n < numCashiers; n++) {
    		Exec("../test/cashier",15);
		}

    	Exec("../test/manager",15);
		
		return;
	}
}
