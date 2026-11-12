#include "major2.h"

int shell_terminal = STDIN_FILENO;
pid_t shell_pgid;
int shell_is_interactive = 0;

// Splits command line on semicolons (;) and executes them sequentially.
void process_command_line(char *line)
{
    char *saveptr;
    char *cmd = strtok_r(line, ";", &saveptr);
    while (cmd != NULL) {
        execute_single_command(cmd);
        cmd = strtok_r(NULL, ";", &saveptr);
    }
}

int main(int argc, char **argv)
{
    FILE *input_stream = stdin;

    // Load the shell's path list from PATH; original PATH is restored on exit (Mark Joseph)
    path_init();

    // Validate argument count
    if (argc > 2) {
        fprintf(stderr, "usage: %s [batchFile]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Determine batch vs. interactive mode
    if (argc == 2) {
        input_stream = fopen(argv[1], "r");
        if (!input_stream) {
            fprintf(stderr, "Error: cannot open batch file %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        shell_is_interactive = 0;
    } else {
        shell_is_interactive = isatty(shell_terminal);
    }

    // Terminal foreground and process group configuration
    if (shell_is_interactive) {
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp())) {
            kill(-shell_pgid, SIGTTIN);
        }

        // Parent shell ignores interactive job signals
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        shell_pgid = getpid();
        if (setpgid(shell_pgid, shell_pgid) < 0) {
            perror("Couldn't put shell into its own process group");
            exit(EXIT_FAILURE);
        }
        tcsetpgrp(shell_terminal, shell_pgid);
    }
    char line_buffer[MAX_LINE_LEN + 2];

    while (1) {
        if (shell_is_interactive) {
            printf("newshell> ");
            fflush(stdout);
        }

        if (fgets(line_buffer, sizeof(line_buffer), input_stream) == NULL) {
            // EOF or Ctrl-D caught
            break;
        }

        // Batch mode: echo read line back to stdout
        if (!shell_is_interactive) {
        printf("%s", line_buffer);
        fflush(stdout);
        }

        // Enforce 512-byte limit
        size_t len = strlen(line_buffer);
        if (len > MAX_LINE_LEN && line_buffer[len - 1] != '\n') {
            fprintf(stderr, "Error: Command line exceeds 512 characters\n");
            int ch;
            while ((ch = fgetc(input_stream)) != '\n' && ch != EOF);
            continue;
        }

        // Trim trailing newline
        if (len > 0 && line_buffer[len - 1] == '\n') {
            line_buffer[len - 1] = '\0';
        }

        process_command_line(line_buffer);
    }

    if (input_stream != stdin) {
        fclose(input_stream);
    }

    return 0;
}
