/*
 * scheduler.c
 *
 *  Created on: Apr 17, 2010
 *      Author: sellek
 */

#include "include/scheduler.h"

#include <stdlib.h>

sched_t* fifo_sched_init();
sched_t* prio_sched_init();

sched_t* sched_init(sched_type type)
{
	switch(type)
	{
	case stFifo:
		return fifo_sched_init();
		break;
	case stPrio:
		return prio_sched_init();
		break;
	default:
		return NULL;
		break;
	};
}

int sched_add_thread(sched_t* schd, thread_t* thrd)
{
	return schd->add_thread(schd, thrd);
}

thread_t* sched_next_thread(sched_t* schd)
{
	return schd->next_thread(schd);
}

void sched_for_all_threads(sched_t* sched, void(*func)(thread_t*))
{
	sched->for_all_threads(sched,func);
}

int sched_destroy(sched_t* scheduler)
{
	return scheduler->destroy(scheduler);
}
