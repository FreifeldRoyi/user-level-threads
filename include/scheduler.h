/*
 * scheduler.h
 *
 *  Created on: Mar 30, 2010
 *      Author: Freifeld Royi
 */
#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "thread.h"

struct sched_t;

/**
 * Defining a new scheduler
 */
struct sched_t* sched_init();

/**
 * Adds a new thread to the scheduler
 */
int sched_add_thread(struct sched_t* schd, thread_t* thrd);

/**
 * Returns the next thread in the scheduler's queue
 */
thread_t* sched_next_thread(struct sched_t* schd);

/**call a function on all scheduled threads. with a given argument.
 * */
void sched_for_all_threads(struct sched_t* sched, void(*func)(thread_t*));

/**
 * Deallocate the memory for the scheduler
 */
int sched_destroy(struct sched_t* scheduler);

#endif /* SCHEDULER_H_ */
