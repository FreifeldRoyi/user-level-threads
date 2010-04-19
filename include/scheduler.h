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
#define PRIO_INC(_x) (((_x) > HIGHEST_PRIO)? ((_x)-1): (_x))
#define PRIO_DEC(_x) (((_x) < LOWEST_PRIO)? ((_x)+1): (_x))


/**
 * Basic scheduler structure
 * Enable a dynamic scheduler type change using the sched_type enum defined above.
 * It's basically a naive Object implementation =)
 */
typedef struct sched_t
{
	//pointers to the designated functions
	int (*add_thread)(struct sched_t*, thread_t*);
	thread_t* (*next_thread)(struct sched_t*);
	void (*for_all_threads)(struct sched_t* sched, void(*func)(thread_t*)); //applies func on each thread in sched
	int (*destroy)(struct sched_t*); //de-allocate

	void* sched; //used as a pointer to the structure of the scheduler's real struct
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
