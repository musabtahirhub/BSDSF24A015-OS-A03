#include "shell.h"

// Background jobs: a slot is free when its pid is 0.
static pid_t job_pid[MAX_JOBS];
static char* job_cmd[MAX_JOBS];

static void add_job(pid_t pid, const char* cmd) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_pid[i] == 0) {
            job_pid[i] = pid;
            job_cmd[i] = strdup(cmd);
            printf("[%d] %d\n", i + 1, pid);
            return;
        }
    }
    fprintf(stderr, "too many background jobs, not tracking pid %d\n", pid);
}

// Non-blocking: reaps every finished child (no zombies) and drops it from the job list.
void reap_jobs(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (job_pid[i] == pid) {
                printf("[%d] Done    %s\n", i + 1, job_cmd[i]);
                free(job_cmd[i]);
                job_pid[i] = 0;
            }
        }
    }
}

void print_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++)
        if (job_pid[i] != 0)
            printf("[%d] %d  Running  %s\n", i + 1, job_pid[i], job_cmd[i]);
}

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
// Returns the exit status of the last command (0 for background jobs).
int execute(char* arglist[], int background) {
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

    if (background) {             // don't wait; reap_jobs() collects it later
        add_job(pids[ncmds - 1], arglist[0]);
        return 0;
    }

    int status = 0;
    for (int c = 0; c < ncmds; c++)
        waitpid(pids[c], &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
