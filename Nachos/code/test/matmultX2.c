/* matmult.c 
 *    Test program to do matrix multiplication on large arrays.
 *
 *    Intended to stress virtual memory system.
 *
 *    Ideally, we could read the matrices off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];


void matmult() {


	int i, j, k;
	int m1;
	int val;



	Write("Fork called mat_mult!\n",22,ConsoleOutput);

	for (i = 0; i < Dim; i++)		/* first initialize the matrices */
		for (j = 0; j < Dim; j++) {
			A[i][j] = i;
			B[i][j] = j;
			C[i][j] = 0;
		}

	Write("Initialized\n",12,ConsoleOutput);

	for (i = 0; i < Dim; i++)		/* then multiply them together */
		for (j = 0; j < Dim; j++)
			for (k = 0; k < Dim; k++)
				C[i][j] += A[i][k] * B[k][j];

	Write("Mat_mult done!\n",15,ConsoleOutput);

	Printf("Result: %d\n",11,C[Dim-1][Dim-1]*100000,0);
	/*Release(l1);*/

	/*return;*/
	Exit(C[Dim-1][Dim-1]);		/* and then we're done */
}


int
main()
{

	Exec("../test/matmult",15);
	Exec("../test/matmult",15);

	/*Easier to do the execing, since then we don't need to worry about giving each thread it's own
	 * unique matrices, which are too big to fit on a single thread's stack.*/
	/*Fork((void*)matmult);
	Fork((void*)matmult);*/

	Exit(0);


}
