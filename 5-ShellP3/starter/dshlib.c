#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

// Handles built-in commands, such as exit or cd
int handle_builtin_cmd(cmd_buff_t *cmd)
{
    if (strcmp(cmd->argv[0], EXIT_CMD) == 0)
    {
        exit(0);
    }
    else if (strcmp(cmd->argv[0], "cd") == 0)
    {
        if (cmd->argc > 1)
        {
            if (chdir(cmd->argv[1]) != 0)
            {
                perror("cd");
            }
        }
        return OK;
    }
    return -1;
}

// Sets up the command buffer and ensures the input is null-terminated.
// Returns an error code if an improper command is passed.
int parse_input(const char *input, cmd_buff_t *cmd) {
    char buffer[SH_CMD_MAX];
    strncpy(buffer, input, SH_CMD_MAX - 1);
    buffer[SH_CMD_MAX - 1] = '\0';  

    cmd->argc = 0;
    cmd->_cmd_buffer = strdup(buffer);

    char *ptr = cmd->_cmd_buffer;
    while (*ptr) {
        while (*ptr == ' ') ptr++;
        if (*ptr == '\0') break;

        if (*ptr == '"' || *ptr == '\'') {
            char quote = *ptr++;
            cmd->argv[cmd->argc] = ptr;
            while (*ptr && *ptr != quote) ptr++;
            if (*ptr) *ptr++ = '\0';  
            else {
                return ERR_EXEC_CMD; 
            }
        } else {
            cmd->argv[cmd->argc] = ptr;
            while (*ptr && *ptr != ' ') ptr++;
            if (*ptr) *ptr++ = '\0';  
        }

        cmd->argc++;
        if (cmd->argc >= CMD_ARGV_MAX) break;  
    }

    cmd->argv[cmd->argc] = NULL;  

    return (cmd->argc == 0) ? WARN_NO_CMDS : OK;
}

// Performs the fork and exec commands, forking a new process and executing the child process.
// Returns the appropriate error code if an error occurs.
int fork_and_exec_cmd(cmd_buff_t *cmd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return ERR_MEMORY;
    } else if (pid == 0) {
        if (execvp(cmd->argv[0], cmd->argv) == -1) {
            return ERR_EXEC_CMD;
        }
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return ERR_MEMORY;
        }
    }
    return OK;
}

// Main loop that waits for the user to input commands and then executes them until the exit command is given.
int exec_local_cmd_loop()
{
    char cmd_buff[SH_CMD_MAX];
    cmd_buff_t cmd;
    int rc;

    while (1)
    {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        rc = parse_input(cmd_buff, &cmd);
        if (rc == WARN_NO_CMDS)
        {
            printf("%s\n", CMD_WARN_NO_CMD);
            continue;
        }
        else if (rc != OK)
        {
            fprintf(stderr, "Error: %d\n", rc);
            continue;
        }

        if (handle_builtin_cmd(&cmd) == OK)
        {
            continue;
        }

        rc = fork_and_exec_cmd(&cmd);
        if (rc != OK)
        {
            return ERR_EXEC_CMD;
        }
    }

    return OK;
}
