/*
 * thread.c
 *
 *  Created on: April 8, 2010
 *      Author: Freifeld Royi
 */

#include "include/thread.h"
#include "include/scheduler.h"
#include <stdlib.h>
#include <assert.h>

#define MAX_STACK_SIZE 4000
#define MAX_THREAD_COUNT 4096

typedef struct _manager_thread_params_t
{
	struct sched_t* sched;
}manager_thread_params_t;

typedef struct _global_stats_t
{
	unsigned switches;
}global_stats_t;

thread_t* cur_thread = NULL;
thread_t* manager_thread = NULL;
thread_t* thread_container[MAX_THREAD_COUNT];
int next_thread = -1;
global_stats_t global_stats = {0};

/* save machine context */
#define mctx_save(_uctx) (void)getcontext(&_uctx)

/* restore machine context */
#define mctx_restore(_uctx) (void)setcontext(_uctx)

/* create machine context which can later be used to save & restore threads */
void mctx_create(ucontext_t *uctx, void (*sf_addr)( ), void *sf_arg, void *sk_addr, size_t sk_size)
{
        /* fetch current context */
        getcontext(uctx);
        /* adjust to new context */
        uctx->uc_link = NULL;
        uctx->uc_stack.ss_sp = sk_addr;
        uctx->uc_stack.ss_size = sk_size;
        uctx->uc_stack.ss_flags = 0;
        /* make new context */
        makecontext(uctx, sf_addr, 1, sf_arg);
        return;
}

int create_thread(void(*sf_addr)(), void* sf_arg)
{
	int toReturn = -1;
	ucontext_t* cont = NULL;

	if (next_thread < MAX_THREAD_COUNT)
	{
		thread_t* toAdd = (thread_t*)malloc(sizeof(thread_t));
		size_t size = MAX_STACK_SIZE;
		assert(toAdd != NULL);
		toReturn = next_thread;
		mctx_create(cont, sf_addr, sf_arg, malloc(MAX_STACK_SIZE),size);
		toAdd -> cont = *cont;
		toAdd -> state = tsFinished;
		toAdd -> stats.max_switches_wait = 0;
		toAdd -> stats.cur_switches_wait = 0;
		toAdd -> ID = next_thread;
		thread_container[next_thread] = toAdd;
		++next_thread;
	}

	return toReturn;
}

void
thread_yield(int pInfo, int statInfo)
{
	cur_thread->state = tsFinished;
	mctx_save(cur_thread->cont);
/* When this thread is started again, this is the point where the execution will commence.
 * We need to distinguish between the situation where we are yielding and the situation
 * when we yielded and now started running again.
 */
	if (cur_thread->state == tsRunning)
	{//we yielded and now continue.
		return;
	}
	else
	{
		mctx_restore(&manager_thread->cont);
	}
}

void update_switch_count(thread_t* thread)
{
	++thread->stats.cur_switches_wait;
	if (thread->stats.cur_switches_wait > thread->stats.max_switches_wait)
	{
		thread->stats.max_switches_wait = thread->stats.cur_switches_wait;
	}
}

void
manager_thread_func(void* ptr)
{
	manager_thread_params_t* param = (manager_thread_params_t*)ptr;

	while (1)
	{
		while ( (cur_thread = sched_next_thread(param->sched)) == NULL)
		{
			/*if there is no thread to run, wait until there is...*/
			///TODO this is probably wrong. the manager should exit here.
		}

		//the thread to run is not part of the scheduler now. we update the stats
		//of all the others.
		sched_for_all_threads(param->sched, &update_switch_count);
		++global_stats.switches;

		cur_thread->state = tsRunning;
		mctx_save(manager_thread->cont);

	/* now check if we got here because the other thread restored our context after
	 * it yielded, or if it didn't get a chance to run yet.
	 */
		if (cur_thread->state == tsRunning)
		{
			mctx_restore(&cur_thread->cont);
		}
		else
		{
			//return the thread that yielded to the scheduler.
			sched_add_thread(param->sched, cur_thread);
		}
	}
}

unsigned thread_stats(unsigned request_stats)
{
	unsigned tid;
	if (request_stats & THREAD_NONGLOBAL_STATS)
	{
		tid = request_stats & ~THREAD_NONGLOBAL_STATS;
		///TODO find the specific stats for the thread with the proper tid.
		return 0;
	}

	switch(request_stats)
	{
	case THREAD_STAT_MAX_SWITCHES:
	{
		///TODO go over all threads and find the maximal value
		return 0;
	}break;
	case THREAD_STAT_TOTAL_SWITCHES:
	{
		return global_stats.switches;
	}break;
	default:
		assert(0);
	}
}

void thread_manager_init(void* arg)
{
	if (next_thread <= -1)
	{
		next_thread = 0;
	}

	/* TODO Do your magic here Tom*/
}

int current_thread_id()
{
	int toReturn = -1;

	if (cur_thread != NULL)
		toReturn = cur_thread -> ID;

	return toReturn;
}
