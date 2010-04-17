/*
 * test_app.h
 *
 *  Created on: April 16, 2010
 *      Author: Freifeld Royi
 */

#include "include/test_app.h"
#include "../include/app_main.h"

void run_app_tests()
{
	//test_load_app_data();
	test_do_run();
}

void test_load_app_data()
{
	printf("Testing load app data\nProblems are: ");
	FILE* f = fopen("test.txt","r");
	app_data_t toTest = load_app_data(f);
	int i;

	if (toTest.ntasks != 4)
		printf(" TASK_NO_ERR");
	if (toTest.nthreads != 5)
		printf(" THREAD_NO_ERR");

	for (i = 0; i < 4; ++i)
		assert(toTest.tasks[i].task_id == i);
	if (!(toTest.tasks[0].ndeps = 2))
		printf(" WRONG_DEP0");
	if (!(toTest.tasks[1].ndeps = 1))
		printf(" WRONG_DEP1");
	if (!(toTest.tasks[2].ndeps = 1))
		printf(" WRONG_DEP2");
	if (!(toTest.tasks[3].ndeps = 0))
		printf(" WRONG_DEP3");


	if (!toTest.thread_params[0].ntasks == 2)
		printf( " WRONG_TASKS_T0");
	if (!toTest.thread_params[1].ntasks == 2)
		printf( " WRONG_TASKS_T1");
	if (!toTest.thread_params[2].ntasks == 1)
		printf( " WRONG_TASKS_T2");
	if (!toTest.thread_params[3].ntasks == 0)
		printf( " WRONG_TASKS_T3");
	if (!toTest.thread_params[4].ntasks == 3)
		printf( " WRONG_TASKS_T4");

	printf("\n");
}

void test_do_run()
{
	//ui_cmd_t cmd = get_command();
	FILE* f = fopen("test.txt","r");
	app_data_t dat = load_app_data(f);
	BOOL b = do_run(NULL,&dat);

	printf("%u",b);
}


