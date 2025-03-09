
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"

/**** 
 **** FOR REMOTE SHELL USE YOUR SOLUTION FROM SHELL PART 3 HERE
 **** THE MAIN FUNCTION CALLS THIS ONE AS ITS ENTRY POINT TO
 **** EXECUTE THE SHELL LOCALLY
 ****
 */

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

 
// Handle built-in commands like 'exit' and 'cd'
int handle_builtin_cmd(cmd_buff_t *cmd) {
    if (strcmp(cmd->argv[0], EXIT_CMD) == 0) {
        exit(0);
    } else if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argc > 1) {
            if (chdir(cmd->argv[1]) != 0) {
                perror("cd");
            }
        }
        return OK;
    }
    return -1;
}

// Parse a single command into cmd_buff_t
int parse_single_cmd(const char *cmd, cmd_buff_t *cmd_buff) {
    char buffer[SH_CMD_MAX];
    strncpy(buffer, cmd, SH_CMD_MAX - 1);
    buffer[SH_CMD_MAX - 1] = '\0';

    cmd_buff->argc = 0;
    cmd_buff->_cmd_buffer = strdup(buffer);
    if (!cmd_buff->_cmd_buffer) return ERR_MEMORY;

    char *ptr = cmd_buff->_cmd_buffer;
    while (*ptr) {
        while (*ptr == ' ') ptr++;
        if (*ptr == '\0') break;

        if (*ptr == '"' || *ptr == '\'') {
            char quote = *ptr++;
            cmd_buff->argv[cmd_buff->argc] = ptr;
            while (*ptr && *ptr != quote) ptr++;
            if (*ptr) *ptr++ = '\0';
            else return ERR_EXEC_CMD;
        } else {
            cmd_buff->argv[cmd_buff->argc] = ptr;
            while (*ptr && *ptr != ' ') ptr++;
            if (*ptr) *ptr++ = '\0';
        }

        cmd_buff->argc++;
        if (cmd_buff->argc >= CMD_ARGV_MAX) break;
    }

    cmd_buff->argv[cmd_buff->argc] = NULL;
    return (cmd_buff->argc == 0) ? WARN_NO_CMDS : OK;
}

// Parse input command into a list of commands (handles pipes)
int parse_input(const char *input, command_list_t *cmd_list) {
    char *cmd;
    int i = 0;
    char *cmd_line_copy = strdup(input);
    if (!cmd_line_copy) return ERR_MEMORY;

    cmd = strtok(cmd_line_copy, PIPE_STRING);
    while (cmd && i < CMD_MAX) {
        if (parse_single_cmd(cmd, &cmd_list->commands[i]) != OK) {
            free(cmd_line_copy);
            return ERR_EXEC_CMD;
        }
        i++;
        cmd = strtok(NULL, PIPE_STRING);
    }

    free(cmd_line_copy);
    cmd_list->num = i;
    return (i == 0) ? WARN_NO_CMDS : OK;
}

// Fork and execute a single command with optional pipe redirection
int fork_and_exec_cmd(cmd_buff_t *cmd, int in_fd, int out_fd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return ERR_MEMORY;
    } else if (pid == 0) { 
        if (in_fd != STDIN_FILENO) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        
        execvp(cmd->argv[0], cmd->argv);
        perror("execvp");
        exit(ERR_EXEC_CMD);
    }
    return pid;
}

// Execute a pipeline of commands
int execute_pipeline(command_list_t *clist) {
    int pipefd[2];
    int in_fd = STDIN_FILENO;
    pid_t pids[CMD_MAX];
    int i;

    for (i = 0; i < clist->num; ++i) {
        if (i < clist->num - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                return ERR_MEMORY;
            }
        }

        pids[i] = fork_and_exec_cmd(&clist->commands[i], in_fd, (i < clist->num - 1) ? pipefd[1] : STDOUT_FILENO);
        if (pids[i] < 0) return ERR_MEMORY;

        if (in_fd != STDIN_FILENO) close(in_fd);
        if (i < clist->num - 1) close(pipefd[1]);
        in_fd = pipefd[0];
    }

    for (i = 0; i < clist->num; ++i) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            return ERR_EXEC_CMD;
        }
    }
    return OK;
}

// Main loop for command input and execution
int exec_local_cmd_loop() {
    char cmd_buff[SH_CMD_MAX];
    command_list_t cmd_list;
    int rc;

    while (1) {
        printf("%s", SH_PROMPT);
        if (!fgets(cmd_buff, SH_CMD_MAX, stdin)) {
            printf("\n");
            break;
        }
        
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        rc = parse_input(cmd_buff, &cmd_list);
        
        if (rc == WARN_NO_CMDS) {
            printf("%s\n", CMD_WARN_NO_CMD);
            continue;
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        } else if (rc != OK) {
            fprintf(stderr, "Error: %d\n", rc);
            continue;
        }

        rc = execute_pipeline(&cmd_list);
        if (rc != OK) {
            fprintf(stderr, "Error: %d\n", rc);
        }
    }
    return OK;
}
