#include "include/app_util.h"
#include "include/scheduler.h"

#ifndef APP_MAIN_H_
#define APP_MAIN_H_

#define MAX_CMD_LEN 4
#define PROMPT "> "

typedef struct _ui_cmd_t{
	char command[MAX_CMD_LEN+1];
	char param[FILENAME_MAX];
}ui_cmd_t;

/**Display a prompt and read a command from the user.
 * The command that was read is stored in a ui_cmd_t - the first
 * non-whitespace sequence is stored as cmd.command and the second as cmd.param.
 * Any further input is not read.
 *
 * @return the strings that were read.
 *
 * */
ui_cmd_t get_command();

/**perform a 'load' UI command as defined in the assignment.
 * @param cmd 	the UI input that caused this call. cmd.param is assumed
 * 				to contain the name of the file to load.
 * @param app_data 	a pointer to a struct that will be filled with the data
 * 					loaded from the file.
 * @return TRUE if the function was successful, FALSE otherwise.
 * */
BOOL do_load(ui_cmd_t* cmd, app_data_t* app_data);

/**perform a 'run' UI command.
 * @param cmd 	the UI input that caused this call. cmd.param is assumed
 * 				to contain a number representing the requested scheduling policy.
 * @param app_data 	a pointer to a struct that will be used. If it's threads have
 * 					not been initialized, they will be.
 * @return TRUE if the function was successful, FALSE otherwise.
 * */
BOOL do_run(ui_cmd_t* cmd, app_data_t* app_data);

/**perform an 'SW' UI command.
 * @param cmd 	the UI input that caused this call. cmd.param is assumed
 * 				to contain a number representing the thread id.
 * @param app_data 	the app's state.
 * @return TRUE if the function was successful, FALSE otherwise.
 * */
BOOL do_sw(ui_cmd_t* cmd, app_data_t* app_data);

/**perform an 'MSW' UI command.
 * @param cmd 	the UI input that caused this call. cmd.param is ignored.
 * @param app_data 	the app's state.
 * @return TRUE if the function was successful, FALSE otherwise.
 * */
BOOL do_msw(ui_cmd_t* cmd, app_data_t* app_data);

/**perform an 'ASW' UI command.
 * @param cmd 	the UI input that caused this call. cmd.param is ignored.
 * @param app_data 	the app's state.
 * @return TRUE if the function was successful, FALSE otherwise.
 * */
BOOL do_asw(ui_cmd_t* cmd, app_data_t* app_data);

/**perform a 'switches' UI command.
 * @param cmd 	the UI input that caused this call. cmd.param is ignored.
 * @param app_data 	the app's state.
 * @return TRUE if the function was successful, FALSE otherwise.
 * */
BOOL do_switches(ui_cmd_t* cmd, app_data_t* app_data);

/**perform a 'JW' UI command.
 * @param cmd 	the UI input that caused this call. cmd.param is assumed
 * 				to contain a number representing the thread id.
 * @param app_data 	the app's state.
 * @return TRUE if the function was successful, FALSE otherwise.
 * */
BOOL do_jw(ui_cmd_t* cmd, app_data_t* app_data);

/**perform an 'MJW' UI command.
 * @param cmd 	the UI input that caused this call. cmd.param is ignored.
 * @param app_data 	the app's state.
 * @return TRUE if the function was successful, FALSE otherwise.
 * */
BOOL do_mjw(ui_cmd_t* cmd, app_data_t* app_data);

/**perform an 'AJW' UI command.
 * @param cmd 	the UI input that caused this call. cmd.param is ignored.
 * @param app_data 	the app's state.
 * @return TRUE if the function was successful, FALSE otherwise.
 * */
BOOL do_ajw(ui_cmd_t* cmd, app_data_t* app_data);

/**perform a 'tasks' UI command.
 * @param cmd 	the UI input that caused this call. cmd.param is ignored.
 * @param app_data 	the app's state.
 * @return TRUE if the function was successful, FALSE otherwise.
 * */
BOOL do_tasks(ui_cmd_t* cmd, app_data_t* app_data);

/**the main function of the app.
 * */
int app_main(int argc, char **argv);

#endif /* APP_MAIN_H_ */
