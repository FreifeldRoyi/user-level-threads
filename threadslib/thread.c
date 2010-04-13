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
#include <string.h>
#include <errno.h>

#include <stdio.h>

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

static thread_t* cur_thread = NULL;
static thread_t* manager_thread = NULL;
static thread_t* thread_container[MAX_THREAD_COUNT];
static global_stats_t global_stats = {0};
static struct sched_t* sched = NULL;
static ucontext_t return_context;

/* save machine context */
#define mctx_save(_uctx) assert(getcontext(&_uctx) >= 0)

/* restore machine context */
#define mctx_restore(_uctx) assert(setcontext(_uctx) >= 0)

/* create machine context which can later be used to save & restore threads */
void mctx_create(ucontext_t *uctx, void (*sf_addr)( ), void *sf_arg, void *sk_addr, size_t sk_size)
{
        /* fetch current context */
        getcontext(uctx);
        /* adjust to new context */
        uctx->uc_link = sf_addr; ///TODO why is this assignment needed?
        uctx->uc_stack.ss_sp = sk_addr;
        uctx->uc_stack.ss_size = sk_size;
        uctx->uc_stack.ss_flags = 0;
        /* make new context */
        makecontext(uctx, sf_addr, 1, sf_arg);
        assert(errno == 0);
        return;
}

static thread_t*
allocate_thread(void(*sf_addr)(), void* sf_arg)
{
	thread_t* ret = (thread_t*)malloc(sizeof(thread_t));

	assert(ret != NULL);
	mctx_create(&ret->cont, sf_addr, sf_arg, malloc(MAX_STACK_SIZE),MAX_STACK_SIZE);
	ret -> state = tsFinished;
	ret -> stats.max_switches_wait = 0;
	ret -> stats.cur_switches_wait = 0;

	return ret;
}

int create_thread(void(*sf_addr)(), void* sf_arg)
{
	int tid = 0;

	while ((tid < MAX_THREAD_COUNT) && (thread_container[tid] != NULL))
	{
		++tid;
	}
	if (tid == MAX_THREAD_COUNT)
		return -1;

	thread_t* toAdd = allocate_thread(sf_addr, sf_arg);
	toAdd->ID = tid;
	thread_container[tid] = toAdd;

	sched_add_thread(sched, toAdd);

	return tid;
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

	printf("manager thread started.\n");
	while (1)
	{
		while ( (cur_thread = sched_next_thread(param->sched)) == NULL)
		{
			manager_thread->state = tsFinished;
			mctx_restore(&return_context);
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
			printf("manager: restoring thread %d\n", cur_thread->ID);
			mctx_restore(&cur_thread->cont);
		}
		else
		{
			if (thread_container[cur_thread->ID] == NULL)
			{
				//the current thread called thread_term and is marked for removal.
				free(cur_thread);
				cur_thread = NULL;
			}
			else
			{
				//return the thread that yielded to the scheduler.
				sched_add_thread(param->sched, cur_thread);
			}
		}
	}
}

unsigned thread_stats(unsigned request_stats)
{
	unsigned tid;
	if (request_stats & THREAD_NONGLOBAL_STATS)
	{
		tid = request_stats & ~THREAD_NONGLOBAL_STATS;
		assert(thread_container[tid]!=NULL);
		return thread_container[tid]->stats.max_switches_wait;
	}

	switch(request_stats)
	{
	case THREAD_STAT_MAX_SWITCHES:
	{
		unsigned max = 0;
		for (tid = 0; tid<MAX_THREAD_COUNT; ++tid)
		{
			if ((thread_container[tid] != NULL)&&(max < thread_container[tid]->stats.max_switches_wait))
				max = thread_container[tid]->stats.max_switches_wait;
		}
		return max;
	}break;
	case THREAD_STAT_TOTAL_SWITCHES:
	{
		return global_stats.switches;
	}break;
	default:
		assert(0);
	}
}

///TODO need to get a context to return to when the manager thread is done.
void thread_manager_init(void* arg)
{
	manager_thread_params_t* params = malloc(sizeof(manager_thread_params_t));

	memset(thread_container, 0, MAX_THREAD_COUNT*sizeof(thread_t*));

	sched = arg;
	params->sched = arg;
	manager_thread = allocate_thread(manager_thread_func, params);
}

void threads_start()
{
	global_stats.switches = 0;
	manager_thread->state = tsRunning;
	mctx_save(return_context);
/*When the manager thread is done it will set it's state to tsFinished and
 * restore our context.
 * */
	if (manager_thread->state == tsRunning)
	{
		mctx_restore(&manager_thread->cont);
	}
	else
	{
		return;
	}
}

void thread_term()
{
	thread_container[cur_thread->ID] = NULL;
	thread_yield(0,0);
}

int current_thread_id()
{
	int toReturn = -1;

	if (cur_thread != NULL)
		toReturn = cur_thread -> ID;

	return toReturn;
}
