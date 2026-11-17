#include "shell.h"

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D
    }
    
    cmdline[pos] = '\0';
    return cmdline;
}

char** tokenize(char* cmdline) {
    // Edge case: empty command line
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; // Skip leading whitespace
        
        if (*cp == '\0') break; // Line was only whitespace

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) { // No arguments were parsed
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

// Circular buffer: command number n (1-based) lives in history[(n-1) % HISTORY_SIZE].
static char* history[HISTORY_SIZE];
static int hist_count = 0;   // total commands ever added

void add_to_history(const char* cmd) {
    int idx = hist_count % HISTORY_SIZE;
    free(history[idx]);          // overwrite the oldest when full
    history[idx] = strdup(cmd);
    hist_count++;
}

// Returns command number n, or NULL if it is out of range / already overwritten.
char* get_history(int n) {
    if (n < 1 || n > hist_count || n <= hist_count - HISTORY_SIZE)
        return NULL;
    return history[(n - 1) % HISTORY_SIZE];
}

static void print_history(void) {
    int start = hist_count > HISTORY_SIZE ? hist_count - HISTORY_SIZE : 0;
    for (int i = start; i < hist_count; i++)
        printf("%d  %s\n", i + 1, history[i % HISTORY_SIZE]);
}

// Returns 1 if arglist[0] was a built-in (and runs it), 0 otherwise.
int handle_builtin(char** arglist) {
    if (strcmp(arglist[0], "exit") == 0) {
        exit(0);
    }
    if (strcmp(arglist[0], "cd") == 0) {
        char* dir = arglist[1] ? arglist[1] : getenv("HOME");
        if (dir == NULL || chdir(dir) != 0)
            perror("cd");
        return 1;
    }
    if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <dir>   change directory\n");
        printf("  exit       exit the shell\n");
        printf("  help       show this help\n");
        printf("  history    show command history\n");
        printf("  jobs       list background jobs\n");
        printf("  !n         re-run command number n\n");
        return 1;
    }
    if (strcmp(arglist[0], "history") == 0) {
        print_history();
        return 1;
    }
    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }
    return 0;
}
