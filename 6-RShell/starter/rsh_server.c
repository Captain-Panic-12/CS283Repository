#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"

/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threaded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 */
int start_server(char *ifaces, int port, int is_threaded) {
    int svr_socket;
    int rc;

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0) {
        return svr_socket; 
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);

    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The server socket.
 *   
 *  This function stops the server.
 */
int stop_server(int svr_socket) {
    return close(svr_socket);
}



/*
 * boot_server(ifaces, port)
 *      ifaces: The interface to bind to.
 *      port:   The port to bind to.
 *   
 *  This function boots the server.
 */
int boot_server(char *ifaces, int port) {
    int svr_socket;
    int ret;
    struct sockaddr_in addr;

    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    int enable = 1;
    if (setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ifaces, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    ret = bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket: The server socket.
 *   
 *  This function processes client requests.
 */
int process_cli_requests(int svr_socket) {
    int cli_socket;
    int rc = OK;
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    while (1) {
        cli_socket = accept(svr_socket, (struct sockaddr *)&cli_addr, &cli_len);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }

        rc = exec_client_requests(cli_socket);
        if (rc == OK_EXIT) {
            break; 
        }

        close(cli_socket);
    }

    return rc;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket: The client socket.
 *   
 *  This function executes client requests.
 */
int exec_client_requests(int cli_socket) {
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (!io_buff) {
        return ERR_RDSH_SERVER; 
    }

    while (1) {
        ssize_t recv_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0);
        if (recv_size < 0) {
            perror("recv");
            send_message_string(cli_socket, "Error: Communication failure");
            free(io_buff);
            return ERR_RDSH_COMMUNICATION; 
        }
        if (recv_size == 0) {
            fprintf(stderr, "Client disconnected\n");
            break;
        }

        io_buff[recv_size - 1] = '\0';

        if (strlen(io_buff) >= RDSH_COMM_BUFF_SZ) {
            send_message_string(cli_socket, "Error: Command too large");
            send_message_eof(cli_socket);
            continue; 
        }

        command_list_t cmd_list;

        if (parse_input(io_buff, &cmd_list) == OK) {
            int exit_code = rsh_execute_pipeline(cli_socket, &cmd_list);

            if (exit_code != OK) {
                send_message_string(cli_socket, "Error: Command execution failed");
                send_message_eof(cli_socket);
            } else {
                send_message_string(cli_socket, "Command executed successfully");
                send_message_eof(cli_socket);
            }
        } else {
            send_message_string(cli_socket, "Error: Invalid command");
            send_message_eof(cli_socket);
        }

        if (strcmp(io_buff, "exit") == 0) {
            break;
        } else if (strcmp(io_buff, "stop-server") == 0) {
            send_message_string(cli_socket, "Server stopping...");
            send_message_eof(cli_socket);
            free(io_buff);
            return OK_EXIT;
        }
    }

    free(io_buff);
    return OK;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket: The client socket.
 *   
 *  This function sends the EOF character to the client.
 */
int send_message_eof(int cli_socket) {
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket: The client socket.
 *      buff:       The message to send.
 *   
 *  This function sends a message to the client.
 */
int send_message_string(int cli_socket, char *buff) {
    int send_len = strlen(buff) + 1; 
    int sent_len = send(cli_socket, buff, send_len, 0);

    if (sent_len != send_len) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock: The client socket.
 *      clist:    The command list.
 *   
 *  This function executes the command pipeline.
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];  
    pid_t pids[clist->num];
    int pids_st[clist->num];     
    int exit_code;

    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == 0) { 
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            } else {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            }

            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
          //  perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }

    exit_code = WEXITSTATUS(pids_st[clist->num - 1]);
    return exit_code;
}