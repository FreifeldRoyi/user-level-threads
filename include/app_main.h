/*
 * app_main.h
 *
 *  Created on: April 16, 2010
 *      Author: Freifeld Royi
 */

#include "include/app_util.h"
#include "include/scheduler.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef APP_MAIN_H_
#define APP_MAIN_H_

#define MAX_CMD_LEN 4
#define PROMPT "> "

typedef struct _ui_cmd_t{
	char command[MAX_CMD_LEN+1];
	char param[FILENAME_MAX];
}ui_cmd_t;

app_data_t load_app_data(FILE* f);

ui_cmd_t get_command();

BOOL do_load(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_run(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_sw(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_msw(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_asw(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_switches(ui_cmd_t* cmd, app_data_t* app_data);


BOOL do_jw(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_mjw(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_ajw(ui_cmd_t* cmd, app_data_t* app_data);

BOOL do_tasks(ui_cmd_t* cmd, app_data_t* app_data);

int app_main(int argc, char **argv);

#endif /* APP_MAIN_H_ */