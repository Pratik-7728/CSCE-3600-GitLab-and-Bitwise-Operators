
/*
 * CSCE 3600 - Major Assignment 2
 * Name: Radheshyam Shrestha
 * Section: 003
 * Date: November 9, 2026
 * Description: Input and output redirection for the shell.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

/*
 * Redirect standard input or output to a file.
 * Call this function in the child process before execvp().
 *
 * filename: file used for redirection
 * mode: '<' for input or '>' for output
 *
 * Returns 0 on success and -1 on failure.
 */
int setup_redirection(const char *filename, char mode)
{
    int fd;
    int target_fd;

    if (filename == NULL || filename[0] == '\0') {
        fprintf(stderr, "redirection: missing filename\n");
        return -1;
    }

    if (mode == '<') {
        fd = open(filename, O_RDONLY);
        target_fd = STDIN_FILENO;
    }
    else if (mode == '>') {
        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        target_fd = STDOUT_FILENO;
    }
    else {
        fprintf(stderr, "redirection: invalid operator\n");
        return -1;
    }

    if (fd == -1) {
        perror("redirection: open");
        return -1;
    }

    if (dup2(fd, target_fd) == -1) {
        perror("redirection: dup2");
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        perror("redirection: close");
        return -1;
    }

    return 0;
}
