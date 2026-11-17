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

// True if s (after leading blanks) starts with the whole word `word`.
static int starts_with_word(const char* s, const char* word) {
    size_t n = strlen(word);
    s += strspn(s, " \t");
    return strncmp(s, word, n) == 0 && (s[n] == '\0' || s[n] == ' ' || s[n] == '\t');
}

// first_line is "if <command>". Keeps reading lines up to "fi", storing the
// then/else blocks, then runs one block depending on <command>'s exit status.
// shortcut: no nested if inside a block; upgrade by making this recursive.
static int run_if(char* first_line) {
    char* cond = first_line + strspn(first_line, " \t") + 2;   // text after "if"
    char* then_block[MAX_BLOCK];
    char* else_block[MAX_BLOCK];
    int nthen = 0, nelse = 0;
    int part = 0;          // 0 = before then, 1 = then-block, 2 = else-block
    int found_fi = 0;
    char* line;

    while (!found_fi && (line = readline("> ")) != NULL) {
        char* w = line + strspn(line, " \t");
        if (starts_with_word(w, "fi"))        found_fi = 1;
        else if (starts_with_word(w, "then")) part = 1;
        else if (starts_with_word(w, "else")) part = 2;
        else if (part == 1 && nthen < MAX_BLOCK) then_block[nthen++] = strdup(w);
        else if (part == 2 && nelse < MAX_BLOCK) else_block[nelse++] = strdup(w);
        free(line);
    }

    int status = 0;
    if (!found_fi || part == 0) {
        fprintf(stderr, "syntax error: expected if ... then ... fi\n");
        status = 1;
    } else if (run_line(cond) == 0) {          // exit status 0 = success -> then
        for (int i = 0; i < nthen; i++) status = run_line(then_block[i]);
    } else {
        for (int i = 0; i < nelse; i++) status = run_line(else_block[i]);
    }

    for (int i = 0; i < nthen; i++) free(then_block[i]);
    for (int i = 0; i < nelse; i++) free(else_block[i]);
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

        if (starts_with_word(cmdline, "if"))
            run_if(cmdline);
        else
            run_line(cmdline);
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
