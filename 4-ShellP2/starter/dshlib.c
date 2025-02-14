#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

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

// As the name suggests, this handles built in cmds, such as exit or cd. 
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
 
 // This function sets up the command buffer, and ensures the input is null terminated.
 // If an improper command is passed, then it returns an error code. 
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
// This function performs the fork and exec commands, by forking a new process and then executing the child process. 
// If an error occurs, then it returns the approriate error code. 
int fork_and_exec_cmd(cmd_buff_t *cmd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return ERR_MEMORY;
    } else if (pid == 0) {
        if (execvp(cmd->argv[0], cmd->argv) == -1) {
            return ERR_EXEC_CMD
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
}

 // This code acts a loop, waiting for the user to input the prebuilt commands and then execute them, until told to exit. 
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
 