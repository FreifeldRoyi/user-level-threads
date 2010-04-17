/*
 * scheduler.h
 *
 *  Created on: Mar 30, 2010
 *      Author: Freifeld Royi
 */
#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "thread.h"

typedef enum{stFifo, stPrio} sched_type;

/*NOTE: priorities are reversed - 0 is the highest priority.
 */
#define LOWEST_PRIO 16
#define HIGHEST_PRIO 0
#define PRIO_INC(_x) if ((_x) > HIGHEST_PRIO) --(_x)
#define PRIO_DEC(_x) if ((_x) < LOWEST_PRIO) ++(_x)

typedef struct sched_t
{
	int (*add_thread)(struct sched_t*, thread_t*);
	thread_t* (*next_thread)(struct sched_t*);
	void (*for_all_threads)(struct sched_t* sched, void(*func)(thread_t*));
	int (*destroy)(struct sched_t*);

	void* sched;
}sched_t;

/**
 * Defining a new scheduler
 */
sched_t* sched_init(sched_type type);

/**
 * Adds a new thread to the scheduler
 */
int sched_add_thread(struct sched_t* schd, thread_t* thrd);

/**
 * Returns the next thread in the scheduler's queue
 */
thread_t* sched_next_thread(struct sched_t* schd);

/**call a function on all scheduled threads.
 * */
void sched_for_all_threads(struct sched_t* sched, void(*func)(thread_t*));

/**
 * Deallocate the memory for the scheduler
 */
int sched_destroy(struct sched_t* scheduler);

#endif /* SCHEDULER_H_ */
