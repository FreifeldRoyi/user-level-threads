/*
 * prio_scheduler.c
 *
 *  Created on: April 17, 2010
 *      Author: Freifeld Royi
 */

#include "include/scheduler.h"
#include "include/queue.h"
#include <assert.h>

#define SCHED(x) ((prio_sched_t*) ((x)->sched))
#define SCHED_QUEUE(x) (SCHED((x)) -> prio_sched)

typedef struct _prio_sched_t
{
	// TODO decide whether or not we want some sort of limit to the queue's priority
	queue_t* prio_sched[LOWEST_PRIO];
} prio_sched_t;

int prio_sched_add_thread(struct sched_t* schd, thread_t* thrd)
{
	assert(thrd->prio < LOWEST_PRIO);
	return queue_push(SCHED_QUEUE(schd)[thrd->prio], thrd);
}

thread_t* prio_sched_next_thread(struct sched_t* schd)
{
	thread_t* toReturn = NULL;
	queue_t** schd_queue = SCHED_QUEUE(schd);
	int i = 0;

	while ((i<LOWEST_PRIO) && (queue_size(schd_queue[i]) == 0))
		++i;

	if (i < LOWEST_PRIO)
		toReturn = THREAD(queue_pop(schd_queue[i]));

	return toReturn;
}

void prio_sched_for_all_threads(struct sched_t* sched, void(*func)(thread_t*))
{
	int i;
	for (i = 0; i < LOWEST_PRIO; ++i)
	{
		node_t* node = QUEUE_HEAD(SCHED_QUEUE(sched)[i]);
		while (node != NULL)
		{
			func(NODE_DATA(node));
			node = NODE_NEXT(node);
		}
	}
}

int prio_sched_destroy(struct sched_t* scheduler)
{
	int toReturn = 0, i;
	queue_t** schd_queue = SCHED_QUEUE(scheduler);

	for (i = 0; i < LOWEST_PRIO; ++i)
	{
		if (queue_size(schd_queue[i]) == 0)
		{
			queue_destroy(schd_queue[i]);
			schd_queue[i] = NULL;
		}
		else
		{
			toReturn = -1;
		}
	}

	free(SCHED(scheduler));
	free(scheduler);

	return toReturn;
}

sched_t* prio_sched_init()
{
	sched_t* toReturn = (sched_t*)malloc(sizeof(sched_t));
	int i;
	assert(toReturn != NULL);

	toReturn->sched = malloc(sizeof(prio_sched_t));
	for (i = 0; i < LOWEST_PRIO; ++i)
	{
		SCHED_QUEUE(toReturn)[i] =	queue_init();
	}

	toReturn->add_thread = prio_sched_add_thread;
	toReturn->next_thread = prio_sched_next_thread;
	toReturn->for_all_threads = prio_sched_for_all_threads;
	toReturn->destroy = prio_sched_destroy;

	return toReturn;
}
