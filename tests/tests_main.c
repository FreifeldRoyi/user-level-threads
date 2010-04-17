/*
 * tests_main.c
 *
 *  Created on: Apr 17, 2010
 *      Author: freifeldroyi
 */
#include "include/tests_main.h"
#include "include/test_threads.h"
#include "include/test_queue.h"
#include "include/test_app.h"


void queue_tests()
{
	run_queue_tests();
}

void priority_shced_tests()
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
