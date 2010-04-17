#include "include/app_util.h"

#include <assert.h>
#include <stdio.h>
#include <malloc.h>

void
free_app_data(app_data_t* app_data)
{
	int i;

	sched_destroy(app_data->sched);

	for (i=0; i < app_data->ntasks; ++i)
	{
		free(app_data->tasks[i].deps);
	}
	for (i=0; i < app_data->nthreads; ++i)
	{
		free(app_data->thread_params[i].my_tasks);
	}
	free(app_data->tasks);
	free(app_data->thread_params);
}

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
  int my_thread_id = current_thread_id()+1;
  unsigned i;
  unsigned job_count;
  unsigned job_count_diff;
  unsigned completed;
  unsigned initial_prio = params->prio;

  printf("***Thread %d running with %d tasks\n",my_thread_id, params->ntasks);

  while (!done)
  {
	completed = 0;
    done = TRUE;
    for (i=0; i<params->ntasks; ++i)
    {
      printf("***Thread %d checking task %d\n",my_thread_id, my_tasks[i]->task_id+1);
      if (my_tasks[i]->done)
      {
    	  printf("***task %d already done\n", my_tasks[i]->task_id+1);
      }
      else if ( ready_to_run(my_tasks[i]))
      {
		  printf("Thread %d performed job %d\n", my_thread_id,my_tasks[i]->task_id+1);
		  ++(*params->global_job_count);
		  my_tasks[i]->done = TRUE;
		  ++completed;
      }
      done = done && (my_tasks[i]->done);
    }

    if (completed == 0)
    {
    	PRIO_DEC(params->prio);
    }
    else
    {
    	params->prio = initial_prio;
    }

    job_count = *params->global_job_count;
    thread_yield(params->prio,0);
    job_count_diff = *params->global_job_count - job_count;
    if (job_count_diff > params->job_wait)
    {
    	params->job_wait = job_count_diff;
    }
  }
  printf("Thread %d completed all jobs\n",my_thread_id);

  thread_term();
}
