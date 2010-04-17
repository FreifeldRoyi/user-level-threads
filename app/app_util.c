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

/*
 * spSpecial scheduling policy description:
 * the more dependencies a thread's job has, the lower it's priority will be (because more threads
 * have to run before it in order for it to have a possibility to perform any work).
 * also, to avoid starvation, we'll add the number of times this thread has run before to it's priority,
 * so that at some point some other thread will run.
 */

static unsigned
calculate_prio(worker_thread_params_t* thread_params, int completed)
{
	switch(*thread_params->policy)
	{
	case spDefault: /*...is the same as spFifo*/
	case spFifo:
		return 0;
		break;
	case spPrio:
	{
		if (completed == 0)
		{
			return PRIO_DEC(thread_params->cur_prio);
		}
		else
		{
			return thread_params->orig_prio;
		}
	}break;
	case spSpecial:
	{
		assert(thread_params->ntasks == 1);
		return thread_params->my_tasks[0]->ndeps + thread_params->nyields;
	}break;
	default:
		assert(FALSE);
	}
	return 0; //will never get here - see the assert in the default case.
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

  params->nyields = 0;

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

    params->cur_prio = calculate_prio(params, completed );

    job_count = *params->global_job_count;
    thread_yield(params->cur_prio,0);
    ++params->nyields;
    job_count_diff = *params->global_job_count - job_count;
    if (job_count_diff > params->job_wait)
    {
    	params->job_wait = job_count_diff;
    }
  }
  printf("Thread %d completed all jobs\n",my_thread_id);

  thread_term();
}
