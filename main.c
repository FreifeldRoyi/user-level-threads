/*
 * main.c
 *
 *  Created on: Mar 26, 2010
 *      Author: Freifeld Royi
 */
#include "tests/include/test_threads.h"
#include "tests/include/test_queue.h"
#include "tests/include/test_app.h"
#include "include/app_main.h"
#include <stdio.h>
#include <assert.h>



int main(int argc, char* argv[])
{
	app_main(argc,argv);

	return 0;
}

#ifdef TESTS
void queue_tests()
{
	run_queue_tests();
}

void priority_sched_tests()
{
	run_prio_scheduler_tests();
}

void threads_tests()
{
	run_thread_test();
}

void app_test_cases()
{
	run_app_tests();
}
#endif
