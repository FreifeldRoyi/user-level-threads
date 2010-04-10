/*
 * scheduler.c
 *
 *  Created on: April 8, 2010
 *      Author: Freifeld Royi
 */

#include "include/scheduler.h"
#include "include/queue.h"
#include <assert.h>

#define SCHED(x) ((sched_t*) (x))
#define SCHED_QUEUE(x) SCHED((x)) -> fifo_sched

typedef struct sched_t
{
	queue_t* fifo_sched;
} sched_t;

void sched_for_all_threads(sched_t* sched, void(*func)(thread_t*))
{
	///TODO this is disgusting. we need to add a queue_for_all_nodes function.
	node_t* node = sched->fifo_sched->head;
	while (node != NULL)
	{
		func(node->data);
		node = node->next;
	}
}

struct sched_t* sched_init()
{
	sched_t* toReturn = (sched_t*)malloc(sizeof(sched_t));
	assert(toReturn != NULL);
	SCHED_QUEUE(toReturn) = queue_init();

	return toReturn;
}

int sched_add_thread(struct sched_t* schd, thread_t* thrd)
{
	return queue_push(SCHED_QUEUE(schd), thrd);
}

/**
 * TODO will return NULL if the queue is empty don't forget to mention it in documentation
 */
thread_t* sched_next_thread(struct sched_t* schd)
{
	return THREAD(queue_pop(SCHED_QUEUE(schd)));
}

/**
 * TODO will not delete the scheduler if not empty
 */
int sched_destroy(struct sched_t* scheduler)
{
	int toReturn = SCHED_QUEUE(scheduler);

	if (toReturn == 0)
		free(scheduler);

	return toReturn;
}
