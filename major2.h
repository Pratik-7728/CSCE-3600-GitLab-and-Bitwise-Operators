/*
 * Course: CSCE 3600 - Systems Programming
 * Assignment: Major Assignment 2 - THe Shell and System Calls
 * File: major2.h
 * Description: Shared header file containing constants, structs, and function prototypes.
*/

#ifndef MAJOR2_H
#define MAJOR2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>

#define MAX_LINE_LEN 512
#define MAX_ARGS 64
#define MAX_ALIASES 100

// Alias structure
typedef struct {
    char name[64];
    char command[MAX_LINE_LEN];
} Alias;

// Global alias table and shell terminal state
extern Alias alias_table[MAX_ALIASES];
extern int alias_count;
extern int shell_terminal;
extern pid_t shell_pgid;
extern int shell_is_interactive;

// Built-in functions
int handle_builtin(char **args);
int builtin_exit(char **args);
int builtin_alias(char **args);
int builtin_cd(char **args);     // builtin.c (Radheshyam Shrestha)
int builtin_path(char **args);   // path.c (Mark Joseph)
void path_init(void);            // path.c (Mark Joseph)

// Redirection function
int setup_redirection(const char *filename, char mode); // redirection.c (Radheshyam Shrestha)

// Execution functions
void process_command_line(char *line);
void execute_single_command(char *cmd_str);
int execute_pipeline(char **pipe_cmds, int num_cmds);
int execute_simple_command(char **args, char *input_file, char *output_file);

// Utility functions
char *trim_whitespace(char *str);
int parse_arguments(char *cmd, char **args);

#endif
