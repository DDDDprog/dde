#ifndef SHELL_H
#define SHELL_H

#include "types.h"

#define MAX_COMMAND_LENGTH 256
#define MAX_ARGS 16
#define MAX_PATH_LENGTH 256

typedef struct {
    char* name;
    char* description;
    int (*function)(int argc, char* argv[]);
} shell_command_t;

/* Shell functions */
void shell_init(void);
void shell_run(void);
int shell_execute(char* command_line);
char* shell_readline(void);
int shell_parse_command(char* command_line, char* argv[]);

/* Built-in commands */
int cmd_help(int argc, char* argv[]);
int cmd_clear(int argc, char* argv[]);
int cmd_echo(int argc, char* argv[]);
int cmd_ps(int argc, char* argv[]);
int cmd_kill(int argc, char* argv[]);
int cmd_ls(int argc, char* argv[]);
int cmd_cat(int argc, char* argv[]);
int cmd_mkdir(int argc, char* argv[]);
int cmd_rmdir(int argc, char* argv[]);
int cmd_cd(int argc, char* argv[]);
int cmd_pwd(int argc, char* argv[]);
int cmd_free(int argc, char* argv[]);
int cmd_uptime(int argc, char* argv[]);
int cmd_uname(int argc, char* argv[]);
int cmd_whoami(int argc, char* argv[]);
int cmd_date(int argc, char* argv[]);
int cmd_reboot(int argc, char* argv[]);
int cmd_shutdown(int argc, char* argv[]);

#endif