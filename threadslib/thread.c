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
	unsigned total_switches;
	unsigned max_switches_wait;
}global_stats_t;

static thread_t* cur_thread = NULL;
static thread_t* manager_thread = NULL;
static thread_t* thread_container[MAX_THREAD_COUNT];
static thread_stats_t stats[MAX_THREAD_COUNT]={{0}};
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
	void* stack = malloc(MAX_STACK_SIZE);

	assert(ret != NULL);
	mctx_create(&ret->cont, sf_addr, sf_arg, stack, MAX_STACK_SIZE);
	ret -> state = tsFinished;
	ret -> stack = stack;

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
	stats[tid].cur_switches_wait = 0;
	stats[tid].max_switches_wait = 0;
	toAdd->prio = 0;

	sched_add_thread(sched, toAdd);

	return tid;
}

void
thread_yield(int pInfo, int statInfo)
{
	cur_thread->state = tsFinished;
	cur_thread->prio = pInfo;
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
	thread_stats_t* st = &stats[thread->ID];
	++st->cur_switches_wait;
	if (st->cur_switches_wait > st->max_switches_wait)
	{
		st->max_switches_wait = st->cur_switches_wait;
	}
	if (st->max_switches_wait > global_stats.max_switches_wait)
	{
		global_stats.max_switches_wait = st->max_switches_wait;
	}
}

static void
free_thread(thread_t* thread)
{
	free(thread->stack);
	free(thread);
}

void
manager_thread_func(void* ptr)
{
	manager_thread_params_t* param = (manager_thread_params_t*)ptr;

//	printf("manager thread started.\n");
	while (1)
	{
		if ( (cur_thread = sched_next_thread(param->sched)) == NULL)
		{
			free(param);
			free_thread(manager_thread);
			manager_thread = NULL;
			mctx_restore(&return_context);
		}

		//the thread to run is not part of the scheduler now. we update the stats
		//of all the others.
		sched_for_all_threads(param->sched, &update_switch_count);
		++global_stats.total_switches;

		cur_thread->state = tsRunning;
		mctx_save(manager_thread->cont);

	/* now check if we got here because the other thread restored our context after
	 * it yielded, or if it didn't get a chance to run yet.
	 */
		if (cur_thread->state == tsRunning)
		{
//			printf("manager: restoring thread %d\n", cur_thread->ID);
			stats[cur_thread->ID].cur_switches_wait = 0;
			mctx_restore(&cur_thread->cont);
		}
		else
		{
			if (thread_container[cur_thread->ID] == NULL)
			{
				//the current thread called thread_term and is marked for removal.
				free_thread(cur_thread);
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
		return stats[tid].max_switches_wait;
	}

	switch(request_stats)
	{
	case THREAD_STAT_MAX_SWITCHES:
	{
		return global_stats.max_switches_wait;
	}break;
	case THREAD_STAT_TOTAL_SWITCHES:
	{
		return global_stats.total_switches;
	}break;
	default:
		assert(0);
	}
}

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
	memset(&global_stats,0,sizeof(global_stats));
	manager_thread->state = tsRunning;
	mctx_save(return_context);
/*When the manager thread is done it will set itself to NULL and restore our context
 * */
	if (manager_thread != NULL)
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
