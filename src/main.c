#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    init_readline();

    // readline() prints the prompt, handles line editing and returns a malloc'd line
    while ((cmdline = readline(PROMPT)) != NULL) {
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

        if ((arglist = tokenize(cmdline)) != NULL) {
            if (!handle_builtin(arglist))   // built-ins run inside the shell, no fork
                execute(arglist);

            // Free the memory allocated by tokenize()
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);
        }
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}