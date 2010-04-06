#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "include/thread.h"

typedef unsigned BOOL;
const BOOL FALSE=0;
const BOOL TRUE=1;

typedef struct _task_t{

  struct _task_t** deps;
  int ndeps;

  BOOL done;

  unsigned task_id;
} task_t;

typedef struct{
  task_t** my_tasks;
  unsigned ntasks;

  unsigned thread_id;
} worker_thread_params_t;

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
  unsigned i;

  printf("thread %d running with %d tasks\n",params->thread_id, params->ntasks);

  while (!done)
  {
    done = TRUE;
    for (i=0; i<params->ntasks; ++i)
    {
      printf("thread %d checking task %d\n",params->thread_id, my_tasks[i]->task_id);
      if (my_tasks[i]->done)
      {
	printf("...task %d already done\n", my_tasks[i]->task_id);
      }
      else if ( ready_to_run(my_tasks[i]))
      {
	printf("...marking task %d done\n", my_tasks[i]->task_id);
	my_tasks[i]->done = TRUE;
      }
      done = done && (my_tasks[i]->done);
    }
    ///TODO what values should we pass here?
    thread_yield(0,0);
  }
  printf("thread %d done.\n",params->thread_id);

  thread_term();
}

typedef struct
{
  unsigned ntasks, nthreads;
  task_t* tasks;
  worker_thread_params_t* thread_params;
}app_data_t;

static void
load_thread_tasks(FILE* f, worker_thread_params_t* thread_params, task_t* tasks, unsigned ntasks)
{
  const unsigned alloc_size = ntasks*5+3;/*we assume that a taskid has up to 4 digits.*/
  char* buf = malloc(alloc_size);
  char* token;
  unsigned thread_id;
  unsigned cur_task;

  memset(buf,0,alloc_size);

  buf = fgets(buf,alloc_size, f);
  assert(buf != NULL);
  assert(buf[alloc_size-1] == 0);

  assert (sscanf(buf, "%u : ", &thread_id) == 1);

  buf = strchr(buf, ':')+1;

  assert(thread_id == thread_params->thread_id);

  thread_params->ntasks=0;
  token = strtok(buf,",");
  while (token != NULL)
  {
    unsigned task_id;
    if (sscanf(token, " %u ", &task_id) == 0)
      break;/*last element is empty. this is OK.*/

    ++thread_params->ntasks;
    token = strtok(NULL,",");
  }

  thread_params->my_tasks = calloc(thread_params->ntasks, sizeof(task_t*));

  cur_task = 0;
  token = strtok(buf,",");
  while (token != NULL)
  {
    unsigned task_id;
    if (sscanf(token, " %u ", &task_id) == 0)
      break;/*last element is empty. this is OK.*/

    thread_params->my_tasks[cur_task] = &(tasks[task_id]);

    ++cur_task;
    token = strtok(NULL,",");
  }

}

/*load task dependency matrix*/
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
    dep[j] = (tmp)?TRUE:FALSE;
    ++task->ndeps;
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

  fscanf(f, "k = %u\n", &ret.nthreads);
  fscanf(f, "n = %u\n", &ret.ntasks);

  ret.tasks = calloc(ret.ntasks, sizeof(task_t));
  ret.thread_params = calloc(ret.nthreads, sizeof(worker_thread_params_t));

  for (i=0;i<ret.ntasks;++i)
  {
    ret.tasks[i].task_id = i;
    load_task_deps(f, &ret.tasks[i],ret.tasks, ret.ntasks);
  }
  for (i=0;i<ret.nthreads;++i)
  {
    ret.thread_params[i].thread_id = i;
    load_thread_tasks(f, &ret.thread_params[i], ret.tasks, ret.ntasks);
  }

  return ret;
}

int app_main(int argc, char **argv) {
  app_data_t app_data;
  thread_t *threads;
  unsigned i;

  ///TODO actually open a file
  app_data = load_app_data(NULL);
  threads = calloc(app_data.nthreads, sizeof(thread_t));

  for (i=0;i<app_data.nthreads;++i)
  {
    create_thread(worker_thread,&app_data.thread_params[i]);
  }

  return 0;
}
