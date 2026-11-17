#include "shell.h"

// Runs one command (no ';' inside): built-in or external, foreground or background.
// Returns its exit status.
static int run_command(char* cmd) {
    char** arglist = tokenize(cmd);
    if (arglist == NULL)
        return 0;

    int n = 0;
    while (arglist[n] != NULL) n++;

    // A trailing & means: run in the background, don't wait
    int background = 0;
    if (strcmp(arglist[n - 1], "&") == 0) {
        free(arglist[--n]);
        arglist[n] = NULL;
        background = 1;
    }

    int status = 0;
    if (n > 0 && !handle_builtin(arglist))   // built-ins run inside the shell, no fork
        status = execute(arglist, background);

    // Free the memory allocated by tokenize()
    for (int i = 0; arglist[i] != NULL; i++)
        free(arglist[i]);
    free(arglist);
    return status;
}

// Splits a line on ';' and runs the parts one after another.
// Returns the status of the last one.
static int run_line(char* line) {
    char* save;
    int status = 0;
    for (char* part = strtok_r(line, ";", &save); part != NULL; part = strtok_r(NULL, ";", &save))
        status = run_command(part);
    return status;
}

int main() {
    char* cmdline;

    init_readline();

    while (1) {
        reap_jobs();   // collect finished background jobs so they don't stay zombies

        // readline() prints the prompt, handles line editing and returns a malloc'd line
        if ((cmdline = readline(PROMPT)) == NULL)
            break;     // Ctrl+D

        // !n: swap the line for history entry n, before tokenizing or saving it
        if (cmdline[0] == '!') {
            char* old = get_history(atoi(cmdline + 1));
            if (old == NULL) {
                printf("%s: event not found\n", cmdline);
                free(cmdline);
                continue;
            }
            free(cmdline);
            cmdline = strdup(old);
            printf("%s\n", cmdline);
        }
        if (cmdline[0] != '\0') {
            add_to_history(cmdline);   // our list, for history and !n
            add_history(cmdline);      // readline's list, for up/down arrows
        }

        run_line(cmdline);
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
