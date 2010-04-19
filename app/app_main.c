#include "include/app_main.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define UNUSED(_x) if ((_x) != (_x)) {(_x)=(_x);}

ui_cmd_t
get_command(){
	ui_cmd_t ret;
	char sep;

	memset(ret.command, 0, MAX_CMD_LEN+1);
	memset(ret.param, 0, FILENAME_MAX);

	printf(PROMPT);
	scanf("%4s", ret.command);
	sep = getc(stdin);
	if (sep == '\n')
		return ret;

	scanf("%s", ret.param);

	return ret;
}

/** Create the threads that are used in the app. This function must be called before
 * call to start_threads.
 *
 * @param app_data the state of the app
 * */
static void
initialize_threads(app_data_t* app_data)
{
	unsigned i;

	if (app_data->sched == NULL)
	{
		app_data->sched = sched_init(stPrio);
	}

	thread_manager_init(app_data->sched);
	for (i=0;i<app_data->nthreads;++i)
	{
	  app_data->thread_params[i].global_job_count = &app_data->job_count;
	  app_data->thread_params[i].policy = &app_data->policy;
	  create_thread(worker_thread,&app_data->thread_params[i]);
	}

	app_data->initialized = TRUE;
}

BOOL
do_load(ui_cmd_t* cmd, app_data_t* app_data)
{
	FILE* file;

	if (app_data->loaded)
	{
		printf("Already loaded.\n");
		return FALSE;
	}

	file = fopen(cmd->param,"r");
	if (file == NULL)
	{
	  printf("File not found %s\n", cmd->param);
	  return FALSE;
	}

	*app_data = load_app_data(file);
	initialize_threads(app_data);

	fclose(file);
	return TRUE;
}

BOOL
do_run(ui_cmd_t* cmd, app_data_t* app_data)
{
	unsigned i;

	if (!app_data->loaded)
	{
		printf("Must call 'load' before 'run'.\n");
		return FALSE;
	}
	if (!app_data->initialized)
	{
		initialize_threads(app_data);
	}

	app_data->policy = atoi(cmd->param); //if param is empty, 0 is returned which is spDefault.

	app_data->job_count = 0;
	for (i=0;i<app_data->ntasks; ++i)
	{
	  app_data->tasks[i].done = FALSE;
	}
	threads_start();
	printf("All threads terminated.\n");

	app_data->initialized = FALSE;
	return TRUE;
}

/**Return a numeric representation of cmd.param and make sure it's a valid
 * thread ID. If it's not, -1 is returned and the proper error message is printed.
 * */
static int
validate_tid_param(ui_cmd_t* cmd, app_data_t* app_data)
{
	int tid = -1;
	if (strlen(cmd->param) == 0)
	{
		printf("Parameter required.\n");
		return -1;
	}
	tid = atoi(cmd->param);
	if ((tid < 0) || ( tid >= (int)app_data->nthreads))
	{
	  printf("Invalid thread id.\n");
	  return -1;
	}
	return tid;
}

BOOL
do_sw(ui_cmd_t* cmd, app_data_t* app_data)
{
	int tid;
	if (!app_data->initialized)
	{
	  printf("No data file loaded.\n");
	  return FALSE;
	}
	if ((tid = validate_tid_param(cmd, app_data)) < 0)
	{
	  return FALSE;
	}
	printf("%d\n", thread_stats(THREAD_NONGLOBAL_STATS | tid));
	return TRUE;
}

BOOL
do_msw(ui_cmd_t* cmd, app_data_t* app_data)
{
	UNUSED(cmd);
	if (!app_data->initialized)
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
	UNUSED(cmd);
	if (!app_data->initialized)
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
	UNUSED(cmd);
	if (!app_data->initialized)
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
	if (!app_data->initialized)
	{
	  printf("No data file loaded.\n");
	  return FALSE;
	}
	if ((tid = validate_tid_param(cmd, app_data)) < 0)
	{
	  return FALSE;
	}
	printf("%d\n", app_data->thread_params[tid].job_wait);
	return TRUE;
}

BOOL
do_mjw(ui_cmd_t* cmd, app_data_t* app_data)
{
	unsigned max_job_wait = 0;
	unsigned i;
	UNUSED(cmd);

	if (!app_data->initialized)
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
	unsigned i;
	UNUSED(cmd);

	if (!app_data->initialized)
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
	UNUSED(cmd);

	if (!app_data->initialized)
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
  UNUSED(argc);
  UNUSED(argv);

  app_data.tasks = NULL;
  app_data.initialized = FALSE;
  app_data.loaded = FALSE;

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
	  else
	  {
		  printf("Unknown command %s.\n", cmd.command);
	  }
  }while (!exit);

  if (!app_data.initialized)
  {
	  free_app_data(&app_data);
  }

  return 0;
}
