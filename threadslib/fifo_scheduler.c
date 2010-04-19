/*
 * scheduler.c
 *
 *  Created on: April 8, 2010
 *      Author: Freifeld Royi
 */

#include "include/scheduler.h"
#include "include/queue.h"
#include <assert.h>

#define SCHED(x) ((fifo_sched_t*)((x)->sched))
#define SCHED_QUEUE(x) (SCHED((x)) -> fifo_sched)

typedef struct _fifo_sched_t
{
	queue_t* fifo_sched;
} fifo_sched_t;

void fifo_sched_for_all_threads(sched_t* sched, void(*func)(thread_t*))
{
	///TODO this is disgusting. we need to add a queue_for_all_nodes function.
	node_t* node = SCHED_QUEUE(sched)->head;
	while (node != NULL)
	{
		func(node->data);
		node = node->next;
	}
}

int fifo_sched_add_thread(struct sched_t* schd, thread_t* thrd)
{
	return queue_push(SCHED_QUEUE(schd), thrd);
}

/**
 * TODO will return NULL if the queue is empty don't forget to mention it in documentation
 */
thread_t* fifo_sched_next_thread(struct sched_t* schd)
{
	return THREAD(queue_pop(SCHED_QUEUE(schd)));
}

/**
 * TODO will not delete the scheduler if not empty
 */
int fifo_sched_destroy(struct sched_t* scheduler)
{
	int toReturn = SCHED_QUEUE(scheduler)->size;

	if (toReturn == 0)
	{
		queue_destroy(SCHED_QUEUE(scheduler));
		free(SCHED(scheduler));
		free(scheduler);
	}

	return toReturn;
}

struct sched_t* fifo_sched_init()
{
	sched_t* toReturn = (sched_t*)malloc(sizeof(sched_t));
	assert(toReturn != NULL);
	toReturn->add_thread = fifo_sched_add_thread;
	toReturn->for_all_threads = fifo_sched_for_all_threads;
	toReturn->next_thread = fifo_sched_next_thread;
	toReturn->destroy = fifo_sched_destroy;
	toReturn->sched = malloc(sizeof(fifo_sched_t));
	SCHED_QUEUE(toReturn) = queue_init();

	return toReturn;
}
