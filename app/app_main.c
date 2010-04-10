#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "include/app_util.h"

#define MAX_CMD_LEN 4

typedef struct _ui_cmd_t{
	char command[MAX_CMD_LEN+1];
	char param[FILENAME_MAX];
}ui_cmd_t;

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

BOOL
do_load(ui_cmd_t* cmd, app_data_t* app_data)
{
	FILE* file;
	int i;

	assert(app_data->threads == NULL); //we currently don't support loading more then one file.

	file = fopen(cmd->param,"r");
	if (file == NULL)
	{
	  printf("File not found %s\n", cmd->param);
	  return FALSE;
	}
	*app_data = load_app_data(file);
	app_data->threads = calloc(app_data->nthreads, sizeof(thread_t));

	///TODO need to pass something here...
	thread_manager_init(NULL);
	for (i=0;i<app_data->nthreads;++i)
	{
	  app_data->thread_params[i].global_job_count = &app_data->job_count;
	  create_thread(worker_thread,&app_data->thread_params[i]);
	}
	return TRUE;
}

BOOL
do_run(ui_cmd_t* cmd, app_data_t* app_data)
{
	int i;
	if (app_data->threads == NULL)
	{
	  printf("No data file loaded.\n");
	  return FALSE;
	}
	app_data->job_count = 0;
	for (i=0;i<app_data->ntasks; ++i)
	{
	  app_data->tasks[i].done = FALSE;
	}
	threads_start();
	return TRUE;
}

BOOL
do_sw(ui_cmd_t* cmd, app_data_t* app_data)
{
	unsigned tid;
	if (app_data->threads == NULL)
	{
	  printf("No data file loaded.\n");
	  return FALSE;
	}
	tid = atoi(cmd->param);
	if ((tid < 0) || ( tid >= app_data->nthreads))
	{
	  printf("Invalid thread id.\n");
	  return FALSE;;
	}
	printf("%d\n", thread_stats(THREAD_NONGLOBAL_STATS | tid));
	return TRUE;
}

BOOL
do_msw(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (app_data->threads == NULL)
	{
	  printf("No data file loaded.\n");
	  return FALSE;
	}
	printf("%d\n", thread_stats(THREAD_STAT_MAX_SWITCHES));
	return TRUE;
}

BOOL
do_asw(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (app_data->threads == NULL)
	{
	  printf("No data file loaded.\n");
	  return FALSE;
	}
	unsigned total_switches =thread_stats(THREAD_STAT_TOTAL_SWITCHES);
	printf("%f\n", (float)total_switches/(float)app_data->nthreads );
	return TRUE;
}

BOOL
do_switches(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (app_data->threads == NULL)
	{
	  printf("No data file loaded.\n");
	  return FALSE;
	}
	printf("%d\n",thread_stats(THREAD_STAT_TOTAL_SWITCHES));
	return TRUE;
}


BOOL
do_jw(ui_cmd_t* cmd, app_data_t* app_data)
{
	int tid = -1;
	if (app_data->threads == NULL)
	{
	  printf("No data file loaded.\n");
	  return FALSE;;
	}
	tid = atoi(cmd->param);
	if ((tid < 0) || ( tid >= app_data->nthreads))
	{
	  printf("Invalid thread id.\n");
	  return FALSE;;
	}
	printf("%d\n", app_data->thread_params[tid].job_wait);
	return TRUE;
}

BOOL
do_mjw(ui_cmd_t* cmd, app_data_t* app_data)
{
	unsigned max_job_wait = 0;
	int i;

	if (app_data->threads == NULL)
	{
	  printf("No data file loaded.\n");
	  return FALSE;;
	}

	for (i=0; i<app_data->nthreads; ++i)
	{
		max_job_wait = MAX(max_job_wait, app_data->thread_params[i].job_wait);
	}
	printf("%d\n", max_job_wait);
	return TRUE;
}

BOOL
do_ajw(ui_cmd_t* cmd, app_data_t* app_data)
{
	unsigned sum_job_wait = 0;
	int i;

	if (app_data->threads == NULL)
	{
	  printf("No data file loaded.\n");
	  return FALSE;;
	}

	for (i=0; i<app_data->nthreads; ++i)
	{
		sum_job_wait += app_data->thread_params[i].job_wait;
	}
	printf("%f\n", (float)sum_job_wait/(float)app_data->nthreads);
	return TRUE;
}

BOOL
do_tasks(ui_cmd_t* cmd, app_data_t* app_data)
{
	if (app_data->threads == NULL)
	{
	  printf("No data file loaded.\n");
	  return FALSE;;
	}

	printf("%d\n", app_data->job_count);
	return TRUE;
}

int app_main(int argc, char **argv) {
  app_data_t app_data;
  ui_cmd_t cmd;
  BOOL exit = FALSE;

  do{
	  cmd = get_command();
	  if (!strcmp("exit", cmd.command))
	  {
		  exit = TRUE;
	  }
	  else if (!strcmp("load", cmd.command))
	  {
		  if (!do_load(&cmd, &app_data))
			  continue;
	  }
	  else if (!strcmp("run", cmd.command))
	  {
		  if (!do_run(&cmd, &app_data))
			  continue;
	  }
	  else if (!strcmp("SW", cmd.command))
	  {
		  if (!do_sw(&cmd, &app_data))
			  continue;
	  }
	  else if (!strcmp("MSW", cmd.command))
	  {
		  if (!do_msw(&cmd, &app_data))
			  continue;
	  }
	  else if (!strcmp("ASW", cmd.command))
	  {
		  if (!do_asw(&cmd, &app_data))
			  continue;
	  }
	  else if (!strcmp("switches", cmd.command))
	  {
		  if (!do_switches(&cmd, &app_data))
			  continue;
	  }
	  else if (!strcmp("JW", cmd.command))
	  {
		  if (!do_jw(&cmd, &app_data))
			  continue;
	  }
	  else if (!strcmp("MJW", cmd.command))
	  {
		  if (!do_mjw(&cmd, &app_data))
			  continue;
	  }
	  else if (!strcmp("AJW", cmd.command))
	  {
		  if (!do_ajw(&cmd, &app_data))
			  continue;
	  }
	  else if (!strcmp("tasks", cmd.command))
	  {
		  if (!do_tasks(&cmd, &app_data))
			  continue;
	  }
  }while (!exit);

  return 0;
}
