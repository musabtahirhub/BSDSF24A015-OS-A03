# PUFCIT's custom UNIX Shell

A simple command-line interpreter (shell) built in C for an Operating Systems course.

## Quick Start

### Requirements

GNU Readline development files:
```bash
sudo apt install libreadline-dev
```

### Build the Shell

To compile the project, run the `make` command:
```bash
make
```

### Run the Shell

The compiled executable will be located in the `bin/` directory.
```bash
./bin/myshell
```

### Clean the Project

To remove all compiled object files and the final executable:
```bash
make clean
```

## Directory Structure

*   `/src`: All C source code files (`.c`).
*   `/include`: All header files (`.h`).
*   `/bin`: Contains the final compiled executable (`myshell`).
*   `/obj`: Contains intermediate object files (`.o`) created during compilation.
*   `Makefile`: The build script for the project.
## Features

| Tag | Feature |
|-----|---------|
| v1.0-base | Base shell (fork-exec-wait) |
| v2 | Built-ins: `cd`, `exit`, `help`, `jobs` |
| v3 | `history` (last 20) and `!n` re-execution |
| v4 | GNU Readline: line editing, up/down history, Tab completion |
| v5 | I/O redirection `<` `>` and pipes `|` |
| v6 | Chaining `;`, background `&`, `jobs`, zombie reaping |
| v7 | `if ... then ... else ... fi` |
| v8 | Variables `NAME=value`, `$NAME` expansion, `set` |
