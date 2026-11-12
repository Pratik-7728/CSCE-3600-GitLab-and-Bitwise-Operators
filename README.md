# Major_2 Assignment: The Shell and System Calls 
**Course:** CSCE 3600: Systems Programming
**Term:** Fall 2026
**Environment:** UNT CSE CELL (`cell01.cse.eng.unt.edu` - `cell20.cse.eng.unt.edu`)

---------------------------------------------------------------
## Description
This assignment is a command-line interpreter (shell) written in C called newshell that executes user commands using Linux system calls (fork, execvp, waitpid, pipe, dup2, and tcsetpgrp) rather than relying on system().
The shell supports both interactive mode (with a customizable newshell> prompt) and batch mode (reading and echoing commands directly from an input file until EOF). The project engine supports sequential execution of compound commands delimited by semicolons (;), input line bounds checking up to 512 bytes, multi-pipe command pipelines (up to 3 commands / 2 pipes), terminal process group foreground control with signal masking (preventing Ctrl-C and Ctrl-Z from killing the parent shell), custom built-ins, and an alias engine.

Built-in commands implemented:
1. built-in cd
2. built-in exit
3. built-in path
--------------------------------------------------------------

## Group Members and Contribution
| Member Name | UNT EUID / email | Functions / Components |
| :--- | :--- | :--- |
| Pratik Baidhya | pb0974 | Shell engine driver, batch/interactive modes, semicolon sequencing (main.c); pipeline engine, foreground process group management, signal control (execute.c); built-in exit, alias engine (builtin.c) |
| Radheshyam Shrestha | rs1959 | Built-in cd command with $HOME default fallback (builtin.c); I/O redirection (< and >) (execute.c) |
| Mark Joseph | mrj0159 | Built-in path command: show, append (+), and remove (-) pathnames; keeps PATH in sync for execvp() and restores the original PATH on exit (path.c) |

---------------------------------------------------------------
## Project Structure
```text
major_2/
├── Makefile       # Compilation script supporting all and clean targets
├── README.md      # Project documentation, member roles, design overview, and specs
├── major2.h       # Shared header file containing declarations, structs, and prototypes
├── main.c         # Main driver: interactive prompt, batch execution, semicolon tokenizing, signal ignoring
├── builtin.c      # Built-in command implementations: cd, exit, and custom alias engine
└── execute.c      # Command routing: pipelines (2 pipes/3 commands), foreground terminal control, process groups
├── path.c # Built-in path command (Mark Joseph)

----------------------------------------------------------------
## Design Overview
1. Interactive & Batch Parser (main.c): Uses fgets() to ingest input lines up to 512 bytes. Semicolons (;) are tokenized via strtok_r() so multiple commands execute sequentially regardless of prior command return status. Empty tokens or redundant whitespace are discarded gracefully without errors.
2. Terminal & Signal Control (execute.c): The interactive parent shell sets its own process group via setpgid() and claims terminal control using tcsetpgrp(). Terminal signals (SIGINT, SIGTSTP, SIGQUIT) are ignored in the parent shell. Child processes and pipelines are given their own process groups, restored to SIG_DFL, and granted foreground terminal access during execution before returning control to the shell.
3. Pipeline Engine (execute.c): Multi-stage pipelines (up to 3 commands / 2 pipes) allocate descriptor pairs via pipe() and chain stdout to stdin using dup2(). Unused descriptors are closed in parents and children to prevent deadlocks and EOF stalls.
4. Built-in Commands (builtin.c): Direct execution of non-external commands including exit, cd (with chdir()), and an alias system (supporting listing, alias -c, alias -r, and alias name='cmd').

-----------------------------------------------------------------
## Complete Specification & Ambiguities Handled
* Semicolons: Consecutive semicolons (e.g., ; ;) or leading/trailing semicolons are safely skipped without printing error messages.
* Long Commands: Any input command line exceeding 512 characters triggers an error to stderr and flushes the remaining input line before resuming.
* Sequential Exit: When exit is chained with other commands using semicolons, all preceding commands execute and wait for completion before the shell terminates.
