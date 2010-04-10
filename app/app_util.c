#include "include/app_util.h"

#include <assert.h>
#include <stdio.h>

BOOL
ready_to_run(task_t* task)
{
  int i;
  assert(task!=NULL);

  for (i=0; i < task->ndeps; ++i)
  {
    if (!task->deps[i]->done)
      return FALSE;
  }

  return TRUE;
}

void worker_thread(void* p)
{
  worker_thread_params_t* params=p;
  task_t** my_tasks = params->my_tasks;
  BOOL done = FALSE;
  int my_thread_id = current_thread_id();
  unsigned i;
  unsigned job_count;
  unsigned job_count_diff;

  printf("thread %d running with %d tasks\n",my_thread_id, params->ntasks);

  while (!done)
  {
    done = TRUE;
    for (i=0; i<params->ntasks; ++i)
    {
      printf("thread %d checking task %d\n",my_thread_id, my_tasks[i]->task_id);
      if (my_tasks[i]->done)
      {
    	  printf("...task %d already done\n", my_tasks[i]->task_id);
      }
      else if ( ready_to_run(my_tasks[i]))
      {
		  printf("Thread %d performed job %d\n", my_thread_id,my_tasks[i]->task_id);
		  ++(*params->global_job_count);
		  my_tasks[i]->done = TRUE;
      }
      done = done && (my_tasks[i]->done);
    }
    ///TODO what values should we pass here?
    job_count = *params->global_job_count;
    thread_yield(0,0);
    job_count_diff = *params->global_job_count - job_count;
    if (job_count_diff > params->job_wait)
    {
    	params->job_wait = job_count_diff;
    }
  }
  printf("thread %d completed all jobs\n",my_thread_id);

  thread_term();
}