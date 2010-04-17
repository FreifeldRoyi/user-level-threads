

#include "include/app_main.h"

#define DUMP(_x) printf("%s[%d] %s=%d\n",__FILE__,__LINE__, #_x, _x)

int
strcnt(const char* str, char chr)
{
	const char* cur = str;
	int ret = 0;

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

  assert (sscanf(buf, "%u : ", &thread_id) == 1);
  buf = strchr(buf, ':')+1;

  thread_params->ntasks = strcnt(buf,',') + 1;
  if (thread_params->ntasks == 1) //if there were 0 commas...
  {
	  if (sscanf(buf, " %u ", &thread_id)==EOF)
		  thread_params->ntasks = 0;
  }

  thread_params->my_tasks = calloc(thread_params->ntasks, sizeof(task_t*));

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

  return ret;
}

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

BOOL
do_load(ui_cmd_t* cmd, app_data_t* app_data)
{
	FILE* file;
	int i;

	assert(!app_data->initialized); //we don't support loading more then once.

	file = fopen(cmd->param,"r");
	if (file == NULL)
	{
	  printf("File not found %s\n", cmd->param);
	  return FALSE;
	}
	*app_data = load_app_data(file);
	app_data->sched = sched_init(stFifo);

	thread_manager_init(app_data->sched);
	for (i=0;i<app_data->nthreads;++i)
	{
	  app_data->thread_params[i].global_job_count = &app_data->job_count;
	  create_thread(worker_thread,&app_data->thread_params[i]);
	}
	app_data->initialized = TRUE;

	fclose(file);
	return TRUE;
}

BOOL
do_run(ui_cmd_t* cmd, app_data_t* app_data)
{
	int i;
	if (!app_data->initialized)
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
	printf("All threads terminated.\n");
	return TRUE;
}

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
	if ((tid < 0) || ( tid >= app_data->nthreads))
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
	int i;

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
	int i;

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
	if (!app_data->initialized)
	{
	  printf("No data file loaded.\n");
	  return FALSE;;
	}

	printf("%d\n", app_data->job_count);
	return TRUE;
}

int app_main(int argc, char **argv) {
  app_data_t app_data = {0};
  ui_cmd_t cmd;
  BOOL exit = FALSE;

  app_data.tasks = NULL;

  app_data.initialized = FALSE;

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

  free_app_data(&app_data);

  return 0;
}
