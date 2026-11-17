#include "shell.h"

// Runs in the CHILD: applies < and > redirections, then execs the command.
// Never returns.
static void run_child(char** args) {
    char* argv[MAXARGS + 1];
    int n = 0;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0) {
            char* file = args[i + 1];
            if (file == NULL) {
                fprintf(stderr, "syntax error: missing file after %s\n", args[i]);
                exit(1);
            }
            int in = (args[i][0] == '<');
            int fd = in ? open(file, O_RDONLY)
                        : open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror(file);
                exit(1);
            }
            dup2(fd, in ? STDIN_FILENO : STDOUT_FILENO);
            close(fd);
            i++;                      // skip the filename
        } else {
            argv[n++] = args[i];
        }
    }
    argv[n] = NULL;
    if (n == 0)
        exit(0);

    execvp(argv[0], argv);
    perror("Command not found"); // This line runs only if execvp fails
    exit(127);
}

// Runs "cmd1 | cmd2 | ..." (each part may have < and >).
// Returns the exit status of the last command.
int execute(char* arglist[]) {
    // Work on a copy so we can cut it at each "|" without touching the caller's list.
    char* args[MAXARGS + 1];
    char** cmds[MAXARGS];
    int ncmds = 0, i;

    for (i = 0; arglist[i] != NULL; i++)
        args[i] = arglist[i];
    args[i] = NULL;

    cmds[ncmds++] = args;
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;
            cmds[ncmds++] = &args[i + 1];
        }
    }

    pid_t pids[MAXARGS];
    int prev_read = -1;   // read end of the previous pipe, becomes this command's stdin
    int fd[2];

    for (int c = 0; c < ncmds; c++) {
        int last = (c == ncmds - 1);
        if (!last && pipe(fd) < 0) {
            perror("pipe");
            return 1;
        }

        pids[c] = fork();
        if (pids[c] < 0) {
            perror("fork failed");
            exit(1);
        }
        if (pids[c] == 0) {           // Child process
            if (prev_read != -1) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }
            if (!last) {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }
            run_child(cmds[c]);
        }

        // Parent: close the pipe ends it no longer needs, or the reader never sees EOF
        if (prev_read != -1)
            close(prev_read);
        if (!last) {
            close(fd[1]);
            prev_read = fd[0];
        }
    }

    int status = 0;
    for (int c = 0; c < ncmds; c++)
        waitpid(pids[c], &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
