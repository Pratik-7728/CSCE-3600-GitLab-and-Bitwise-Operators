/*
 * Course: CSCE 3600 - Systems Programming
 * Assignment: Major Assignment 2 - The Shell and System Calls
 * File: execute.c
 * Description: Handles command execution, pipelines, I/O redirection calls, and terminal signal groups.
 */

#include "major2.h"

char *trim_whitespace(char *str)
{
    while (*str == ' ' || *str == '\t' || *str == '\n') str++;
    if (*str == 0) return str;

    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n')) end--;
    end[1] = '\0';
    return str;
}

int parse_arguments(char *cmd, char **args)
{
    int count = 0;
    char *token = strtok(cmd, " \t\n");
    while (token != NULL && count < MAX_ARGS - 1) {
        args[count++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[count] = NULL;
    return count;
}

 // Executes a single command with process group, terminal foreground control, and optional I/O redirection via setup_redirection() (redirection.c).
 
int execute_simple_command(char **args, char *input_file, char *output_file)
{
    if (args[0] == NULL) return 0;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork error");
        return -1;
    }

    if (pid == 0) {
        // Child: set new process group ID
        setpgid(0, 0);

        if (shell_is_interactive) {
            tcsetpgrp(shell_terminal, getpid());
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
        }

        // Apply input/output redirection if specified
        if (input_file != NULL) {
            if (setup_redirection(input_file, '<') == -1) {
                exit(EXIT_FAILURE);
            }
        }
        if (output_file != NULL) {
            if (setup_redirection(output_file, '>') == -1) {
                exit(EXIT_FAILURE);
            }
        }

        execvp(args[0], args);
        fprintf(stderr, "%s: command not found\n", args[0]);
        exit(EXIT_FAILURE);
    } else {
        // Parent: set child group and give foreground terminal control
        setpgid(pid, pid);

        if (shell_is_interactive) {
            tcsetpgrp(shell_terminal, pid);
        }

        int status;
        waitpid(pid, &status, WUNTRACED);

        // Parent regains foreground terminal control
        if (shell_is_interactive) {
            tcsetpgrp(shell_terminal, shell_pgid);
        }
    }

    return 0;
}

// Pipelining: SUpports uo to 3 commands/ 2 pipes
int execute_pipeline(char **pipe_cmds, int num_cmds)
{
    int pipefds[2 * (num_cmds - 1)];

    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe creation failed");
            return -1;
        }
    }

    pid_t pids[num_cmds];

    for (int i = 0; i < num_cmds; i++) { 
     pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork error");
            return -1;
        }
        if (pids[i] == 0) {
            if (i == 0) setpgid(0, 0);
            else setpgid(0, pids[0]);

            if (shell_is_interactive) {
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
            }

            if (i != 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i != num_cmds - 1) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }

            for (int j = 0; j < 2 * (num_cmds - 1); j++) {
                close(pipefds[j]);
            }

            char *args[MAX_ARGS];
            parse_arguments(pipe_cmds[i], args);
            if (args[0] == NULL) exit(EXIT_SUCCESS);

            execvp(args[0], args);
            fprintf(stderr, "%s: command not found\n", args[0]);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_cmds; i++) {
        setpgid(pids[i], pids[0]);
    }

    if (shell_is_interactive) {
        tcsetpgrp(shell_terminal, pids[0]);
    }

    for (int i = 0; i < 2 * (num_cmds - 1); i++) {
        close(pipefds[i]);
    }

    for (int i = 0; i < num_cmds; i++) {
        int status;
        waitpid(pids[i], &status, WUNTRACED);
    }

    if (shell_is_interactive) {
        tcsetpgrp(shell_terminal, shell_pgid);
    }

    return 0;
}

// Evaluates command: checks alias substitution, pipeline, built-ins, and execution
void execute_single_command(char *cmd_str)
{
    cmd_str = trim_whitespace(cmd_str);
    if (strlen(cmd_str) == 0) return;

    // Check alias substitution
   char expanded[MAX_LINE_LEN];
    for (int i = 0; i < alias_count; i++) {
        size_t nlen = strlen(alias_table[i].name);
        if (strncmp(cmd_str, alias_table[i].name, nlen) == 0 &&
            (cmd_str[nlen] == '\0' || cmd_str[nlen] == ' ' || cmd_str[nlen] == '\t')) {
            snprintf(expanded, sizeof(expanded), "%s%s", alias_table[i].command, cmd_str + nlen);
            cmd_str = expanded;
            break;
        }
    }

    // 1. Pipeline handling (|)
    if (strchr(cmd_str, '|') != NULL) {
        char *pipe_cmds[4];
        int num_cmds = 0;
        char *token = strtok(cmd_str, "|");
        while (token != NULL && num_cmds < 4) {
            pipe_cmds[num_cmds++] = trim_whitespace(token);
            token = strtok(NULL, "|");
        }

        if (num_cmds > 3) {
            fprintf(stderr, "Error: pipeline supports up to 3 commands\n");
            return;
        }
        execute_pipeline(pipe_cmds, num_cmds);
        return;
    }

    // 2. Argument & redirection parsing
    char *input_file = NULL;
    char *output_file = NULL;
    char *args[MAX_ARGS];
    int arg_count = 0;

    char cmd_copy[MAX_LINE_LEN];
    strncpy(cmd_copy, cmd_str, MAX_LINE_LEN - 1);
    cmd_copy[MAX_LINE_LEN - 1] = '\0';

    char *tok = strtok(cmd_copy, " \t\n");
    while (tok != NULL && arg_count < MAX_ARGS - 1) {
        if (strcmp(tok, "<") == 0) {
            tok = strtok(NULL, " \t\n");
            if (tok == NULL) {
                fprintf(stderr, "redirection: missing input filename\n");
                return;
            }
            input_file = tok;
        } else if (strcmp(tok, ">") == 0) {
            tok = strtok(NULL, " \t\n");
            if (tok == NULL) {
                fprintf(stderr, "redirection: missing output filename\n");
                return;
            }
            output_file = tok;
        } else {
            args[arg_count++] = tok;
        }
        tok = strtok(NULL, " \t\n");
    }
    args[arg_count] = NULL;

    if (args[0] == NULL) return;

    // 3. Built-ins check
    if (handle_builtin(args)) return;
    
    // 4. External execution
    execute_simple_command(args, input_file, output_file);
}
