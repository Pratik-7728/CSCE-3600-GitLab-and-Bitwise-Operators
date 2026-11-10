#include "major2.h"
Alias alias_table[MAX_ALIASES];
int alias_count = 0;




int builtin_cd(char **args)
{

return 1;
}

// Built-in exit: Terminates the shell cleanly
int builtin_exit(char **args)
{
    exit(EXIT_SUCCESS);
    return 1;
}

/*
* Built-in alias:
* - alias                 : prints all active aliases
* - alias -c              : clears all active aliases
* - alias -r <name>       : removes specific alias
* - alias name='command'  : defines or updates an alias
*/
int builtin_alias(char **args)
{
    // Case 1: Display all aliases
    if (args[1] == NULL) {
        for (int i = 0; i < alias_count; i++) {
            printf("alias %s='%s'\n", alias_table[i].name, alias_table[i].command);
        }
        return 1;
    }

    // Case 2: Clear all aliases (-c)
    if (strcmp(args[1], "-c") == 0) {
        alias_count = 0;
        return 1;
    }

    // Case 3: Remove specific alias (-r alias_name)
    if (strcmp(args[1], "-r") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "alias: -r requires an alias name\n");
            return 1;
        }
        int found = -1;
        for (int i = 0; i < alias_count; i++) {
            if (strcmp(alias_table[i].name, args[2]) == 0) {
                found = i;
                break;
            }
        }
        if (found != -1) {
            for (int i = found; i < alias_count - 1; i++) {
                alias_table[i] = alias_table[i + 1];
            }
            alias_count--;
        } else {
            fprintf(stderr, "alias: %s not found\n", args[2]);
        }
        return 1;
    }

    // Case 4: Set alias (format: alias name='command')
    char *eq = strchr(args[1], '=');
    if (!eq) {
        fprintf(stderr, "alias: incorrect format. Use: alias name='command'\n");
        return 1;
    }

    *eq = '\0';
    char *name = args[1];
    char *cmd_start = eq + 1;

    // Remove quotes if present
    if (*cmd_start == '\'' || *cmd_start == '\"') {
         cmd_start++;
         size_t len = strlen(cmd_start);
         if (len > 0 && (cmd_start[len - 1] == '\'' || cmd_start[len - 1] == '\"')) {
             cmd_start[len - 1] = '\0';
         }
    }

    // Update if already exists
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(alias_table[i].name, name) == 0) {
            strncpy(alias_table[i].command, cmd_start, MAX_LINE_LEN - 1);
            alias_table[i].command[MAX_LINE_LEN - 1] = '\0';
            return 1;
        }
    }

    // Add new alias
    if (alias_count < MAX_ALIASES) {
        strncpy(alias_table[alias_count].name, name, 63);
        alias_table[alias_count].name[63] = '\0';
        strncpy(alias_table[alias_count].command, cmd_start, MAX_LINE_LEN - 1);
        alias_table[alias_count].command[MAX_LINE_LEN - 1] = '\0';
        alias_count++;
    } else {
        fprintf(stderr, "alias: table full\n");
    }

    return 1;
}

// Dispatcher for built-in commands
int handle_builtin(char **args)
{
    if (args[0] == NULL)
        return 0;
    
    if (strcmp(args[0], "exit") == 0)
        return builtin_exit(args);
    if (strcmp(args[0], "alias") == 0)
        return builtin_alias(args);
    if (strcmp(args[0], "cd") == 0)
        return builtin_cd(args);

    return 0;
}
