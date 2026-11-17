#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "
#define HISTORY_SIZE 20

// Function prototypes
void init_readline(void);
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char** arglist);
void add_to_history(const char* cmd);
char* get_history(int n);

#endif // SHELL_H
