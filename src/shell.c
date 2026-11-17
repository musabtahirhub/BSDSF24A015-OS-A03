#include "shell.h"

// Tab completion for the first word: executables found in $PATH.
// Readline calls this repeatedly (state 0 = first call) until it returns NULL.
static char* command_generator(const char* text, int state) {
    static char* path_copy = NULL;
    static char* dir;
    static char* save;
    static DIR* dp;
    struct dirent* entry;

    if (state == 0) {
        free(path_copy);
        path_copy = strdup(getenv("PATH") ? getenv("PATH") : "");
        dir = strtok_r(path_copy, ":", &save);
        dp = NULL;
    }
    while (dir != NULL) {
        if (dp == NULL)
            dp = opendir(dir);
        if (dp != NULL) {
            while ((entry = readdir(dp)) != NULL)
                if (strncmp(entry->d_name, text, strlen(text)) == 0)
                    return strdup(entry->d_name);   // readline frees it
            closedir(dp);
            dp = NULL;
        }
        dir = strtok_r(NULL, ":", &save);
    }
    return NULL;
}

// First word -> command names; otherwise (or no command matched) readline's
// default filename completion is used because we return NULL.
static char** shell_completion(const char* text, int start, int end) {
    if (start == 0)
        return rl_completion_matches(text, command_generator);
    return NULL;
}

void init_readline(void) {
    rl_attempted_completion_function = shell_completion;
}

// Splits a line into words. The operators < > | are always separate tokens
// (so "ls>out" works too). Text inside "double quotes" is one word, quotes removed.
char** tokenize(char* cmdline) {
    if (cmdline == NULL)
        return NULL;

    char** arglist = malloc(sizeof(char*) * (MAXARGS + 1));
    char* cp = cmdline;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; // Skip leading whitespace
        if (*cp == '\0') break;                  // Line was only whitespace

        char* tok = malloc(strlen(cp) + 1);      // a token is never longer than the rest of the line
        int len = 0;
        if (strchr(SPECIAL_CHARS, *cp)) {
            tok[len++] = *cp++;                  // operator token
        } else {
            int in_quotes = 0;
            while (*cp != '\0' && (in_quotes || !strchr(" \t" SPECIAL_CHARS, *cp))) {
                if (*cp == '"') in_quotes = !in_quotes;
                else tok[len++] = *cp;
                cp++;
            }
        }
        tok[len] = '\0';
        arglist[argnum++] = tok;
    }

    if (argnum == 0) { // No arguments were parsed
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
