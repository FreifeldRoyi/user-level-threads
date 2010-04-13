/*
 * main.c
 *
 *  Created on: Mar 26, 2010
 *      Author: freifeldroyi
 */
#include "include/queue.h"
#include "tests/include/test_threads.h"

int main()
{
	int nthreads, nyields;
	for (nthreads=0; nthreads<10; ++nthreads)
	{
		for (nyields=0; nyields<10; ++nyields)
		{
			threads_test_case(10,10);
		}
	}
	return 0;
}
