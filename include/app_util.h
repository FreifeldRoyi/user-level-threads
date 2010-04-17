#ifndef APP_UTIL_H_
#define APP_UTIL_H_

#include "thread.h"
#include "scheduler.h"

#include <stdio.h>

#define MAX(_x,_y) (((_x)>(_y))?(_x):(_y))

typedef unsigned BOOL;
#define FALSE 0
#define TRUE 1

typedef enum{spDefault = 0, spFifo = 1, spPrio = 2, spSpecial = 3} sched_policy;

typedef struct _task_t {
  struct _task_t** deps;
  int ndeps;

  BOOL done;

  unsigned task_id;
} task_t;

typedef struct {
  task_t** my_tasks;
  unsigned ntasks;
  unsigned* global_job_count;
  unsigned job_wait;

  unsigned cur_prio;
  unsigned orig_prio;

  unsigned nyields;
  sched_policy *policy;
} worker_thread_params_t;

void worker_thread(void* p);

typedef struct
{
  unsigned ntasks, nthreads;
  task_t* tasks;
  worker_thread_params_t* thread_params;
  struct sched_t* sched;
  sched_policy policy;

  unsigned job_count;

  BOOL initialized;
  BOOL loaded;
}app_data_t;

void free_app_data(app_data_t* app_data);
app_data_t load_app_data(FILE* f);

#endif
