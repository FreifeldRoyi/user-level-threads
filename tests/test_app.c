/*
 * test_app.h
 *
 *  Created on: April 16, 2010
 *      Author: Freifeld Royi
 */

#include "include/test_app.h"
#include "../include/app_main.h"

#include <assert.h>
#include <string.h>

void test_do_run()
{
	ui_cmd_t cmd;
	BOOL b;
	app_data_t app_data;

	strcpy(cmd.command,"load");
	strcpy(cmd.param,"test.txt");

	b = do_load(&cmd, &app_data);
	printf("LOAD: %u\n",b);

	strcpy(cmd.command,"run");
	strcpy(cmd.param,"");
	b = do_run(&cmd,&app_data);
	printf("RUN: %u\n",b);

	free_app_data(&app_data);
}


#define ASSERT_EQUALS(_x, _y) if (_x != _y) {printf("%s != %s (%d != %d)\n",#_x,#_x,_x,_y);}

void test_load_app_data()
{
	FILE* f = fopen("test.txt","r");
	app_data_t toTest;
	int expected_ndeps[]={2,1,1,0};
	int expected_ntasks[]={2,2,1,0,3};
	int i,j;

	assert(f != NULL);
	toTest = load_app_data(f);
	printf("Testing load app data\nProblems are: ");

	if (toTest.ntasks != 4)
		printf(" TASK_NO_ERR");
	if (toTest.nthreads != 5)
		printf(" THREAD_NO_ERR");

	for (i = 0; i < 4; ++i)
	{
		printf("i: %d\n", i);
		ASSERT_EQUALS(toTest.tasks[i].task_id,i);
	}

	for (i=0;i<4;++i)
	{
		printf("i: %d\n", i);
		ASSERT_EQUALS(toTest.tasks[i].ndeps,expected_ndeps[i]);
		for (j=0; j< toTest.tasks[i].ndeps; ++j)
		{
			assert(toTest.tasks[i].deps[j] != NULL);
		}
	}

	for (i=0;i<5;++i)
	{
		printf("i: %d\n", i);
		ASSERT_EQUALS(toTest.thread_params[i].ntasks,expected_ntasks[i]);
		for (j=0; j< toTest.thread_params[i].ntasks; ++j)
		{
			printf("j: %d\n", j);
			assert(toTest.thread_params[i].my_tasks[j] != NULL);
		}
	}

	printf("\n");
}

void run_app_tests()
{
//	get_command();
//	test_load_app_data();
	test_do_run();
}
