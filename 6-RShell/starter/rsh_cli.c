#include <sys/socket.h>
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
 * exec_remote_cmd_loop(server_ip, port)
 *      server_ip:  The server's IP address.
 *      port:       The server's port.
 *   
 *  This function connects to the server, sends commands, and receives responses.
 */
int exec_remote_cmd_loop(char *address, int port) {
    char *cmd_buff = malloc(RDSH_COMM_BUFF_SZ);
    char *rsp_buff = malloc(RDSH_COMM_BUFF_SZ);
    int cli_socket;
    ssize_t io_size;
    int is_eof;

    if (!cmd_buff || !rsp_buff) {
        return client_cleanup(-1, cmd_buff, rsp_buff, ERR_MEMORY);
    }

    cli_socket = start_client(address, port);
    if (cli_socket < 0) {
        return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_CLIENT);
    }

    while (1) {
        printf("dsh4> ");
        if (!fgets(cmd_buff, RDSH_COMM_BUFF_SZ, stdin)) {
            break; 
        }

        
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

       
        io_size = send(cli_socket, cmd_buff, strlen(cmd_buff) + 1, 0);
        if (io_size < 0) {
            perror("send");
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
        }

    
        while ((io_size = recv(cli_socket, rsp_buff, RDSH_COMM_BUFF_SZ, 0)) > 0) {
            is_eof = (rsp_buff[io_size - 1] == RDSH_EOF_CHAR) ? 1 : 0;
            if (is_eof) {
                rsp_buff[io_size - 1] = '\0'; 
            }

            printf("%.*s", (int)io_size, rsp_buff);

            if (is_eof) {
                break;
            }
        }

        if (io_size < 0) {
            perror("recv");
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
        }

       
        if (strcmp(cmd_buff, "exit") == 0 || strcmp(cmd_buff, "stop-server") == 0) {
            break;
        }
    }

    return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
}

/*
 * start_client(server_ip, port)
 *      server_ip:  The server's IP address.
 *      port:       The server's port.
 *   
 *  This function connects to the server.
 */
int start_client(char *server_ip, int port) {
    struct sockaddr_in addr;
    int cli_socket;
    int ret;

    cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_socket < 0) {
        perror("socket");
        return ERR_RDSH_CLIENT;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(cli_socket);
        return ERR_RDSH_CLIENT;
    }

    ret = connect(cli_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("connect");
        close(cli_socket);
        return ERR_RDSH_CLIENT;
    }

    return cli_socket;
}

/*
 * client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc)
 *      cli_socket:   The client socket.
 *      cmd_buff:     The command buffer.
 *      rsp_buff:     The response buffer.
 *      rc:           The return code.
 *   
 *  This function cleans up resources.
 */
int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc) {
    if (cli_socket > 0) {
        close(cli_socket);
    }
    free(cmd_buff);
    free(rsp_buff);
    return rc;
}