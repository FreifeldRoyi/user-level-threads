/*
 * thread.c
 *
 *  Created on: April 8, 2010
 *      Author: Freifeld Royi
 */

#include "include/thread.h"
#include "include/scheduler.h"
#include <stdlib.h>

typedef struct _manager_thread_params_t
{
	struct sched_t* sched;
}manager_thread_params_t;

thread_t* cur_thread;
thread_t* manager_thread;


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

void
manager_thread_func(void* ptr)
{
	manager_thread_params_t* param = (manager_thread_params_t*)ptr;

	while (1)
	{
		while ( (cur_thread = sched_next_thread(param->sched)) == NULL)
		{
			/*if there is no thread to run, wait until there is...*/
		}

		cur_thread->state = tsRunning;
		mctx_save(manager_thread->cont);

	/* now check if we got here because the other thread restored our context after
	 * it yielded, or if it didn't get a chance to run yet.
	 */
		if (cur_thread->state == tsRunning)
		{
			mctx_restore(&cur_thread->cont);
		}
		//otherwise we return to the top of the loop and run another thread.
	}
}

