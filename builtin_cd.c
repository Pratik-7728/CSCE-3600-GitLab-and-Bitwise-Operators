
/*
 * CSCE 3600 - Major Assignment 2
 * Name: Radheshyam Shrestha
 * Section: 003
 * Date: November 9, 2026
 * Description: Built-in cd command for the shell.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Change the shell's current working directory. */
int builtin_cd(int argc, char *argv[])
{
    const char *directory;

    if (argc > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        return -1;
    }

    if (argc == 1) {
        directory = getenv("HOME");

        if (directory == NULL || directory[0] == '\0') {
            fprintf(stderr, "cd: HOME is not set\n");
            return -1;
        }
    } else {
        directory = argv[1];
    }

    if (chdir(directory) == -1) {
        perror("cd");
        return -1;
    }

    return 0;
}
