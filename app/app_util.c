#include "include/app_util.h"

#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

/**Count the appearances of a character in a string.
 *
 * @param str the string. Must not be NULL.
 * @param chr the character to count in str
 *
 * @return the number of times that chr appears in str.
 * */
static int
strcnt(const char* str, char chr)
{
	const char* cur = str;
	int ret = 0;

	assert(str != NULL);

	while (*cur != '\0')
	{
		if ( (*cur) == chr)
		{
			++ret;
		}
		++cur;
	}
	return ret;
}

/**Return the number of items in a comma-separated-values list.
 *@param str the list of items separated by commas.
 *
 *@return the number of items in the list.
 * */
static int
get_num_items_in_csv_list(char* str)
{
	int ret = strcnt(str,',') + 1;
	unsigned thread_id;
	if (ret == 1) //if there were 0 commas...
	{
	  if (sscanf(str, " %u ", &thread_id)==EOF)
		  ret = 0;
	}
	return ret;
}

/**Load the data about which thread may perform which tasks from a file.
 *
 * @param f the file to use
 * @param thread_params the state of this thread
 * @param tasks the global tasks array
 * @param ntasks the number of tasks in the global array
 * */
static void
load_thread_tasks(FILE* f, worker_thread_params_t* thread_params, task_t* tasks, unsigned ntasks)
{
  const unsigned alloc_size = ntasks*5+3;/*we assume that a taskid has up to 4 digits.*/
  char* buf = malloc(alloc_size), *orig_buf = buf;
  char* token;
  unsigned thread_id;
  unsigned cur_task;

  memset(buf,0,alloc_size);

  buf = fgets(buf,alloc_size, f);
  assert(buf != NULL);
  assert(buf[alloc_size-1] == 0);

  //read the thread id and then move buf to point after it.
  assert (sscanf(buf, "%u : ", &thread_id) == 1);
  buf = strchr(buf, ':')+1;

  thread_params->ntasks = get_num_items_in_csv_list(buf);
  thread_params->my_tasks = calloc(thread_params->ntasks, sizeof(task_t*));

  //iterate over the task id list and add them to this thread's list of
  //tasks.
  //cur_task is the next available item in my_tasks.
  cur_task = 0;
  token = strtok(buf,",");
  while (token != NULL)
  {
    unsigned task_id;
    int sscanf_ret = sscanf(token, " %u ", &task_id);
    assert(sscanf_ret != 0);
    if (sscanf_ret == EOF)
    {
      break;/*last element is empty. this is OK.*/
    }
    //the task_id we read from the file is one-based and our array is zero-based.
    --task_id;

    thread_params->my_tasks[cur_task] = &(tasks[task_id]);

    ++cur_task;
    token = strtok(NULL,",");
  }

  free(orig_buf);

}

/**load task dependency matrix from a file.
 *
 * @param f the file to use
 * @param task the task to load the info for
 * @param tasks the global tasks array
 * @param ntasks the number of tasks in the global array
 * */
static void
load_task_deps(FILE* f, task_t* task, task_t* tasks, unsigned ntasks)
{
  BOOL *dep = calloc(ntasks, sizeof(BOOL));
  unsigned cur_dep = 0;
  unsigned j;

  task->ndeps = 0;

  for (j=0;j<ntasks;++j)
  {
    unsigned tmp;

    fscanf(f, " %u ",&tmp);
    assert(tmp<2);
    if (tmp)
    {
    	dep[j] = TRUE;
		++task->ndeps;
    }
    else
    {
    	dep[j] = FALSE;
    }
  }

  task->deps = calloc(task->ndeps, sizeof(task_t*));

  for (j=0;j<ntasks;++j)
  {
    if (dep[j])
    {
      task->deps[cur_dep++] = &(tasks[j]);
    }
  }

  free(dep);
}

app_data_t load_app_data(FILE* f)
{
  app_data_t ret;
  unsigned i;

  assert(f!=NULL);

  fscanf(f, "k = %u\n", &ret.nthreads);
  fscanf(f, "n = %u\n", &ret.ntasks);

  //printf("app has %d threads, and %d tasks",ret.nthreads,ret.ntasks);

  ret.tasks = calloc(ret.ntasks, sizeof(task_t));
  ret.thread_params = calloc(ret.nthreads, sizeof(worker_thread_params_t));

  memset(ret.tasks, 0, ret.ntasks * sizeof(task_t));
  memset(ret.thread_params, 0, ret.nthreads * sizeof(worker_thread_params_t));

  for (i=0;i<ret.ntasks;++i)
  {
    ret.tasks[i].task_id = i;
    load_task_deps(f, &ret.tasks[i],ret.tasks, ret.ntasks);
  }
  for (i=0;i<ret.nthreads;++i)
  {
    load_thread_tasks(f, &ret.thread_params[i], ret.tasks, ret.ntasks);
  }

  ret.loaded = TRUE;

  return ret;
}

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

/*return TRUE if the given task's dependencies have all completed.
 * */
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
/**Calculate the priority that the given thread should yield with.
 * @param thread_params the state of the thread
 * @param completed the number of jobs that this thread completed since the last yield.
 *
 * @return the thread's new priority according to the scheduling policy.
 * */
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

  while (!done)
  {
	completed = 0;
    done = TRUE;
    for (i=0; i<params->ntasks; ++i)
    {
      if ( (!my_tasks[i]->done) && ( ready_to_run(my_tasks[i])) )
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
