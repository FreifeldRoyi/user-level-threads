/*
 * prio_scheduler.c
 *
 *  Created on: April 17, 2010
 *      Author: Freifeld Royi
 */

#include "include/scheduler.h"
#include "include/queue.h"
#include <assert.h>

#define SCHED(x) ((sched_t*) (x))
#define SCHED_QUEUE(x) SCHED((x)) -> prio_queue
#define MAX_PRIO 16

typedef struct sched_t
{
	// TODO decide whether or not we want some sort of limit to the queue's priority
	queue_t* prio_queue;
} sched_t;

struct sched_t* sched_init()
{
	sched_t* toReturn = (sched_t*)malloc(sizeof(sched_t));
	assert(toReturn != NULL);
	SCHED_QUEUE(toReturn) = queue_init();

	return toReturn;
}

int sched_add_thread(struct sched_t* schd, thread_t* thrd)
{

}

thread_t* sched_next_thread(struct sched_t* schd);

void sched_for_all_threads(struct sched_t* sched, void(*func)(thread_t*));

static int del_err_num(int toReturn, int toAdd)
{
	toReturn *= 10;
	if (toReturn == 0)
		toReturn = toAdd;
	else
		toReturn += toAdd;

	return toReturn;
}


int sched_destroy(struct sched_t* scheduler)
{
	int toReturn = 0, i, size = SCHED_QUEUE(scheduler)->size;
	queue_t* schd_queue = SCHED_QUEUE(scheduler);

	for (i = 0; i < size; ++i)
	{
		//TODO deletion!
	}

	return toReturn;
}
