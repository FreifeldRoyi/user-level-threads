/*
 * thread.h
 *
 *  Created on: March 29, 2010
 *      Author: Freifeld Royi
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <ucontext.h>

#define THREAD(x) ((thread_t*) (x))
#define THREAD_STATS(x) THREAD((x)) -> stats

/**
 *used as an indicator for thread's state.
 *Whether it has finished its job or not
 */
enum thread_state_t {tsRunning, tsFinished} ;

#define THREAD_NONGLOBAL_STATS 0x80000000
#define THREAD_STAT_TOTAL_SWITCHES 0x00000001
#define THREAD_STAT_MAX_SWITCHES   0x00000002

typedef struct _thread_stats_t
{
	unsigned max_switches_wait;
	unsigned cur_switches_wait;
}thread_stats_t;

/*
 *thread's struct type.
 */
typedef struct thread
{
	ucontext_t cont;
	enum thread_state_t state; //thread's state (finished or not as defined above)
	int ID;
	int prio;

	void* stack; //designated stack
} thread_t;

/**
 * This function receives as arguments a pointer to the thread’s main function and a pointer to
 * its argument list. The function will allocate the thread and prepare it but not run it. Once
 * the thread’s fields are all set, the thread is inserted into the threads container.
 * The function returns the newly created thread’s id or -1 in case of a failure to create a
 * thread.
 */
int create_thread(void(*sf_addr)(), void* sf_arg);

/**
 * This function saves the current thread’s context and resumes the manager (restores the
 * manager’s context). The argument pInfo is related to the requested priority upon yielding
 * and the statInfo is an argument which relates to the user’s application, and
 * allows for statistics gathering.
 */
void thread_yield(int pInfo, int statInfo);

/**
 * Terminates the thread and transfer’s control to the manager. Note that this is the graceful
 * means to terminate a thread – an alternate means to terminate all threads is by terminating
 * the manager (we assume that the program completes after the manager exits).
 */
void thread_term();

/**
 * Initializes the manager and the thread container data structure.
 */
void thread_manager_init(void* arg);

/**
 * This function starts the user space threads. All it does is simply let the manager run
 * TODO it also needs to zero the global stats.
 */
void threads_start();

/**
 * Returns the ID of the currently running thread.
 */
int current_thread_id();


/**get statistics about a thread or all the threads.
 *
 * to get an individual thread's switches_wait value, you need to
 * OR the thread_id with THREAD_NONGLOBAL_STATS and pass the result as request_stats.
 *
 * otherwise pass either THREAD_STAT_TOTAL_SWITCHES or THREAD_STAT_MAX_SWITCHES to get
 * global stats.
 * */
unsigned thread_stats(unsigned request_stats);

#endif /* THREAD_H_ */
