/*
 * main.c
 *
 *  Created on: Mar 26, 2010
 *      Author: freifeldroyi
 */
#include "include/queue.h"
#include "tests/include/test_threads.h"
#include "include/thread.h"
#include <stdio.h>

int main()
{
	int nthreads, nyields;
	for (nthreads=0; nthreads<10; ++nthreads)
	{
		for (nyields=0; nyields<10; ++nyields)
		{
			threads_test_case(nthreads, nyields);
		}
	}
	printf("total switches: %d, max switches: %d\n", thread_stats(THREAD_STAT_TOTAL_SWITCHES), thread_stats(THREAD_STAT_TOTAL_SWITCHES));
	return 0;
}
