#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "include/thread.h"

typedef unsigned BOOL;
const BOOL FALSE=0;
const BOOL TRUE=1;

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

typedef struct _ui_cmd_t{
#define MAX_CMD_LEN 4
	char command[MAX_CMD_LEN+1];
	char param[FILENAME_MAX];
}ui_cmd_t;

typedef struct
{
  unsigned ntasks, nthreads;
  task_t* tasks;
  worker_thread_params_t* thread_params;
}app_data_t;


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

  return ret;
}

#define PROMPT "> "

ui_cmd_t
get_command(){
	ui_cmd_t ret;

	memset(ret.command, 0, MAX_CMD_LEN+1);
	memset(ret.param, 0, FILENAME_MAX);

	printf(PROMPT);
	scanf("%4s %s", ret.command, ret.param);

	return ret;
}

int app_main(int argc, char **argv) {
  app_data_t app_data;
  thread_t *threads = NULL;
  ui_cmd_t cmd;
  BOOL exit = FALSE;
  unsigned i;
  unsigned job_count = 0;

  do{
	  cmd = get_command();
	  if (!strcmp("exit", cmd.command))
	  {
		  exit = TRUE;
	  }
	  else if (!strcmp("load", cmd.command))
	  {
		  FILE* file;
		  assert(threads == NULL); //we currently don't support loading more then one file.

		  file = fopen(cmd.param,"r");
		  if (file == NULL)
		  {
			  printf("File not found %s\n", cmd.param);
			  continue;
		  }
		  app_data = load_app_data(file);
		  threads = calloc(app_data.nthreads, sizeof(thread_t));

		  ///TODO need to pass something here...
		  thread_manager_init(NULL);
		  for (i=0;i<app_data.nthreads;++i)
		  {
			  app_data.thread_params[i].global_job_count = &job_count;
			  create_thread(worker_thread,&app_data.thread_params[i]);
		  }
	  }
	  else if (!strcmp("run", cmd.command))
	  {
		  if (threads == NULL)
		  {
			  printf("No data file loaded.\n");
			  continue;
		  }
		  for (i=0;i<app_data.ntasks; ++i)
		  {
			  app_data.tasks[i].done = FALSE;
		  }
		  threads_start();
	  }
  }while (!exit);

  return 0;
}
