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
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXARGS 64
#define PROMPT "FCIT> "
#define SPECIAL_CHARS "<>|&"  // characters that form their own token
#define MAX_JOBS 32
#define MAX_BLOCK 32          // max lines in a then/else block
#define HISTORY_SIZE 20

// Function prototypes
void init_readline(void);
char** tokenize(char* cmdline);
int execute(char** arglist, int background);
void reap_jobs(void);
void print_jobs(void);
int handle_builtin(char** arglist);
void expand_vars(char** arglist);
void add_to_history(const char* cmd);
char* get_history(int n);

#endif // SHELL_H
