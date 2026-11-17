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
#define SPECIAL_CHARS "<>|"   // characters that form their own token
#define HISTORY_SIZE 20

// Function prototypes
void init_readline(void);
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char** arglist);
void add_to_history(const char* cmd);
char* get_history(int n);

#endif // SHELL_H
