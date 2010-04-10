#ifndef APP_UTIL_H_
#define APP_UTIL_H_

#include "thread.h"

#define MAX(_x,_y) (((_x)>(_y))?(_x):(_y))

typedef unsigned BOOL;
#define FALSE 0
#define TRUE 1

typedef struct _task_t {
  struct _task_t** deps;
  int ndeps;

  BOOL done;

  unsigned task_id;
} task_t;

typedef struct{
  task_t** my_tasks;
  unsigned ntasks;
  unsigned* global_job_count;
  unsigned job_wait;
} worker_thread_params_t;

void worker_thread(void* p);

typedef struct
{
  unsigned ntasks, nthreads;
  task_t* tasks;
  worker_thread_params_t* thread_params;

  unsigned job_count;
  thread_t* threads;
}app_data_t;

#endif
