/*
 * path.c - Built-in "path" command for newshell
 *
 * Author:  Mark Joseph (mrj0159)
 * Course:  CSCE 3600, Section 003 - Principles of Systems Programming
 * Date:    November 2026
 *
 * Description:
 *   Keeps the shell's pathname list in a dynamic array of strings. The list
 *   starts as the contents of the PATH environment variable when the shell
 *   is launched. Because the shell runs programs with execvp(), every change
 *   to the list is also written back to the real PATH variable with setenv().
 *   The original PATH is saved at startup and restored when the shell exits
 *   (atexit handler covers both the "exit" built-in and EOF / Ctrl-D).
 *
 *   path            -> print the list, colon separated (e.g. /bin:/usr/bin)
 *   path + <dir>    -> append one pathname
 *   path - <dir>    -> remove one pathname
 */

#include "major2.h"

static char **path_list = NULL;      /* array of pathname strings          */
static int path_count = 0;           /* number of pathnames in path_list   */
static int path_capacity = 0;        /* allocated slots in path_list       */
static char *original_path = NULL;   /* PATH value before the shell began  */
static int had_original_path = 0;    /* was PATH set at all at startup?    */

/* Grow the array if it is full. Returns 0 on success, -1 on failure. */
static int path_reserve(void)
{
    if (path_count < path_capacity)
        return 0;

    int new_cap = (path_capacity == 0) ? 16 : path_capacity * 2;
    char **tmp = realloc(path_list, new_cap * sizeof(char *));
    if (tmp == NULL) {
        perror("path: realloc");
        return -1;
    }
    path_list = tmp;
    path_capacity = new_cap;
    return 0;
}

/* Add a copy of dir to the end of the list. Returns 0 on success. */
static int path_push(const char *dir)
{
    if (path_reserve() < 0)
        return -1;

    char *copy = strdup(dir);
    if (copy == NULL) {
        perror("path: strdup");
        return -1;
    }
    path_list[path_count++] = copy;
    return 0;
}

/* Build "a:b:c" from the list. Caller must free. NULL on failure. */
static char *path_join(void)
{
    size_t total = 1;   /* terminating '\0' */
    for (int i = 0; i < path_count; i++)
        total += strlen(path_list[i]) + 1;   /* +1 for ':' */

    char *joined = malloc(total);
    if (joined == NULL) {
        perror("path: malloc");
        return NULL;
    }

    joined[0] = '\0';
    for (int i = 0; i < path_count; i++) {
        if (i > 0)
            strcat(joined, ":");
        strcat(joined, path_list[i]);
    }
    return joined;
}

/* Write the current list to the real PATH so execvp() can use it. */
static void path_sync_env(void)
{
    char *joined = path_join();
    if (joined == NULL)
        return;
    if (setenv("PATH", joined, 1) < 0)
        perror("path: setenv");
    free(joined);
}

/* Restore PATH to its value from before the shell started and free memory.
 * Registered with atexit() so it runs on "exit" and on EOF. */
static void path_restore(void)
{
    if (had_original_path) {
        if (setenv("PATH", original_path, 1) < 0)
            perror("path: setenv");
    } else {
        unsetenv("PATH");
    }

    for (int i = 0; i < path_count; i++)
        free(path_list[i]);
    free(path_list);
    free(original_path);
    path_list = NULL;
    original_path = NULL;
    path_count = path_capacity = 0;
}

/* Load the initial list from PATH. Call once at the start of main(). */
void path_init(void)
{
    const char *env = getenv("PATH");

    if (env != NULL) {
        had_original_path = 1;
        original_path = strdup(env);
        if (original_path == NULL) {
            perror("path: strdup");
            had_original_path = 0;
        }

        /* Split a copy of PATH on ':' (skip empty entries) */
        char *copy = strdup(env);
        if (copy == NULL) {
            perror("path: strdup");
        } else {
            char *saveptr;
            char *dir = strtok_r(copy, ":", &saveptr);
            while (dir != NULL) {
                path_push(dir);
                dir = strtok_r(NULL, ":", &saveptr);
            }
            free(copy);
        }
    }

    if (atexit(path_restore) != 0)
        fprintf(stderr, "path: could not register exit handler\n");
}

/* Built-in path command. Always returns 1 (handled as a built-in). */
int builtin_path(char **args)
{
    /* path  -> display list */
    if (args[1] == NULL) {
        char *joined = path_join();
        if (joined != NULL) {
            printf("%s\n", joined);
            fflush(stdout);
            free(joined);
        }
        return 1;
    }

    /* path + dir / path - dir need exactly one pathname */
    if ((strcmp(args[1], "+") != 0 && strcmp(args[1], "-") != 0) ||
        args[2] == NULL || args[3] != NULL) {
        fprintf(stderr, "path: usage: path | path + <pathname> | path - <pathname>\n");
        return 1;
    }

    /* path + dir -> append */
    if (strcmp(args[1], "+") == 0) {
        if (path_push(args[2]) == 0)
            path_sync_env();
        return 1;
    }

    /* path - dir -> remove */
    for (int i = 0; i < path_count; i++) {
        if (strcmp(path_list[i], args[2]) == 0) {
            free(path_list[i]);
            for (int j = i; j < path_count - 1; j++)
                path_list[j] = path_list[j + 1];
            path_count--;
            path_sync_env();
            return 1;
        }
    }

    fprintf(stderr, "path: %s not found in path\n", args[2]);
    return 1;
}
