#ifndef APP_UTIL_H_
#define APP_UTIL_H_

#include "thread.h"
#include "scheduler.h"

#include <stdio.h>

#define MAX(_x,_y) (((_x)>(_y))?(_x):(_y))

typedef unsigned BOOL;
#define FALSE 0
#define TRUE 1

/**Scheduling policy:
 * spDefault: should behave the same as spFifo.
 * spFifo: FIFO/RR scheduling.
 * spPrio: variable priority as described in part 3 of the assignment
 * spSpecial: variable priority as described in part 4 of the assignment
 * */
typedef enum{spDefault = 0, spFifo = 1, spPrio = 2, spSpecial = 3} sched_policy;

//a node in the dependency graph.
typedef struct _task_t {
  struct _task_t** deps; //the nodes that this node depends on
  int ndeps; //the number of dependencies of this node.

  BOOL done; //is this node's task completed

  unsigned task_id; //the ID of this node
} task_t;

//the state of a job-processing thread
typedef struct {
  task_t** my_tasks; //pointers to the tasks that this thread may perform.
  unsigned ntasks; //number of tasks that this thread may perform
  unsigned* global_job_count; //pointer to a global count of completed jobs
  unsigned job_wait; //the thread's job_wait value

  unsigned cur_prio; //the current priority of the thread
  unsigned orig_prio; //the original priority of the thread.

  unsigned nyields; //the number of times that this thread yielded (required for the spSpecial scheduling)
  sched_policy *policy; //a pointer to the global scheduling policy.
} worker_thread_params_t;

/*a job-processing thread's function.
 *  @param p should be a pointer to a worker_thread_params_t.
 */
void worker_thread(void* p);

//the state of the job-processing app
typedef struct
{
  unsigned ntasks, nthreads; //number of threads and nodes in the tasks graph
  task_t* tasks; //the actual nodes in the task graph
  worker_thread_params_t* thread_params; //the states of the threads
  struct sched_t* sched; //the scheduler to use
  sched_policy policy; //the global scheduling policy

  unsigned job_count; //the global number of completed jobs

  BOOL initialized; //TRUE if the app's threads have been initialized
  BOOL loaded; //TRUE if this struct's data is valid.
}app_data_t;

/**free the app's state.
 * the app_data pointer itself is not freed, only the pointers it contains.
 * */
void free_app_data(app_data_t* app_data);

/**load the data for the app from a given file.
 * this function does not initialize any threads.
 * */
app_data_t load_app_data(FILE* f);

#endif
