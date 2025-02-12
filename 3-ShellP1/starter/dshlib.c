#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "dshlib.h"

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    clist->num = 0;
    memset(clist->commands, 0, sizeof(clist->commands));

    char *token = strtok(cmd_line, PIPE_STRING);
    while (token != NULL)
    {
        if (clist->num >= CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }

        while (isspace((unsigned char)*token)) token++;
        char *end = token + strlen(token) - 1;
        while (end > token && isspace((unsigned char)*end)) end--;
        end[1] = '\0';

        char *arg = token;
        char *exe = strsep(&arg, " ");

        if (strlen(exe) >= EXE_MAX || (arg != NULL && strlen(arg) >= ARG_MAX))
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        strcpy(clist->commands[clist->num].exe, exe);
        if (arg != NULL)
        {
            strcpy(clist->commands[clist->num].args, arg);
        }
        clist->num++;

        token = strtok(NULL, PIPE_STRING);
    }

    // Check if no commands were provided
    if (clist->num == 0)
    {
        return WARN_NO_CMDS;
    }

    return OK;
}
