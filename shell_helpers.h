
#ifndef SHELL_HELPERS_H
#define SHELL_HELPERS_H

/* Built-in cd command: run in the parent shell. */
int builtin_cd(int argc, char *argv[]);

/* Input/output redirection: run before execvp(). */
int setup_redirection(const char *filename, char mode);

#endif
