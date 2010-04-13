#include "tests/include/test_threads.h"
#include "include/thread.h"
#include "include/scheduler.h"
#include <assert.h>
#include <malloc.h>


void test_thread(void* p)
{
	test_thread_param_t* param = (test_thread_param_t*)p;
	int i;

//	printf("thread %d starting. expected thread_id is %d.\n", current_thread_id(), param->thread_id);
	assert(current_thread_id() == param->thread_id);

	for(i=0; i<param->num_yields; ++i)
	{
		assert(*param->global_counter == (current_thread_id() + param->nthreads*i));
		++(*param->global_counter);
//		printf("thread %d yielding.\n", current_thread_id());
		thread_yield(0,0);
	}

//	printf("thread %d terminating. global counter is %d. switches_wait is %d.\n", current_thread_id(), *param->global_counter, thread_stats(THREAD_NONGLOBAL_STATS|current_thread_id()));

	thread_term();
}

void threads_test_case(int nthreads, int nyields)
{
	test_thread_param_t* params = calloc(nthreads, sizeof(test_thread_param_t));
	int counter = 0;
	int i;

	printf("Started test case with %d threads, each yielding %d times.\n", nthreads, nyields);
	for (i=0;i<nthreads;++i)
	{
		params[i].global_counter = &counter;
		params[i].nthreads = nthreads;
		params[i].num_yields = nyields;
		params[i].thread_id = i;
	}
	thread_manager_init(sched_init());

	for (i=0;i<nthreads;++i)
	{
		create_thread(&test_thread, &params[i]);
//		printf("Thread %d created.\n", i);
	}

//	printf("Starting threads.\n");
	threads_start();
//	printf("All threads finished.\n");
}
