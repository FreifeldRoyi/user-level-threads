/*
 * main.c
 *
 *  Created on: Mar 26, 2010
 *      Author: freifeldroyi
 */
#include "include/queue.h"
#include "tests/include/test_threads.h"
#include "tests/include/test_queue.h"
#include "include/thread.h"
#include "tests/include/test_app.h"
#include <stdio.h>
#include <assert.h>

#define MAX_NTHREADS 10
#define MAX_NYIELDS 5

int main()
{
	run_app_tests();

	return 0;
}

void queue_tests()
{
	run_queue_tests();
}

int thread_test()
{
	int nthreads, nyields;
		unsigned total_switches, max_switches;
		unsigned expected_total_switches, expected_max_switches;
		for (nthreads=1; nthreads<=MAX_NTHREADS; ++nthreads)
		{
			for (nyields=0; nyields<MAX_NYIELDS; ++nyields)
			{
				threads_test_case(nthreads, nyields);

				total_switches = thread_stats(THREAD_STAT_TOTAL_SWITCHES);
				max_switches = thread_stats(THREAD_STAT_MAX_SWITCHES);
				/*every thread yields nyields times, and it's termination causes another
				 * context switch.
				 * */
				expected_total_switches = nthreads*(nyields+1);
				expected_max_switches = nthreads-1;
	//			printf("total switches: %d(expected:%d), max switches: %d(expected:%d)\n", total_switches, expected_total_switches, max_switches, expected_max_switches);
				assert( total_switches ==  expected_total_switches);
				assert( max_switches == expected_max_switches);
			}
		}
		return 0;
}
