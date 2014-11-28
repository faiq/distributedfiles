#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include "../serialize/serialize.h"

int main(int argc, char* argv[]) {
    struct sockaddr_in socket_info;
    char hostname[257];
    struct hostent* host;
    int socket_handle;
    int port = 0;
    int c;
    char* mount = NULL;

    while (1) {
        static struct option long_options[] = {
            {"port", required_argument, 0, 'p'},
            {"mount", required_argument, 0, 'm'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long_only(argc, argv, "p:m:", long_options, &option_index);

        if (c == -1)
            break;
        switch (c) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'm':
                mount = optarg;
                break;
            case '?':
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (port == 0 || mount == NULL) {
        printf("Not enough args\n");
        exit(EXIT_FAILURE);
    }

    bzero(&socket_info, sizeof(struct sockaddr_in));

    gethostname(hostname, 256);
    if ((host = gethostbyname(hostname)) == NULL) {
        printf("Error getting hostname\n");
        exit(EXIT_FAILURE);
    }

    if ((socket_handle = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        close(socket_handle);
        exit(EXIT_FAILURE);
    }

    socket_info.sin_family = AF_INET;
    socket_info.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_info.sin_port = htons(port);

    if (bind(socket_handle, (struct sockaddr*)&socket_info, sizeof(struct sockaddr_in)) < 0) {
        close(socket_handle);
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(socket_handle, 1);

    int socket_conn;
    while (1) {
        if ((socket_conn = accept(socket_handle, NULL, NULL)) < 0) {
            close(socket_handle);
            if (errno == EINTR)
                continue;
            perror("accept");
            exit(EXIT_FAILURE);
        }
        switch(fork()) {
            case -1:
                perror("fork");
                close(socket_handle);
                close(socket_conn);
                exit(EXIT_FAILURE);
            case 0:
                while (1) {
                    char buf[4];
                    int rc = recv(socket_conn, buf, 4, 0);
                    if (rc < 4)
                        exit(EXIT_FAILURE);
                    int size = deserialize_int(buf);
                    char* buffer = malloc(size);
                    rc = recv(socket_conn, buffer, size, 0);
                    if (rc < 4)
                        exit(EXIT_FAILURE);
                    int id = buffer[0];
                    char* filename;
                    int fd;
                    int length;
                    void* file;
                    byte_buffer response;
                    switch (id) {
                        case 0:
                            filename = malloc(strlen(mount) + size + 1);
                            strcpy(filename, mount);
                            strcat(filename, "/");
                            strncat(filename, &buffer[1], size - 1);
                            fd = open(filename, O_CREAT);
                            init_buf(9, &response);
                            put_int(5, &response);
                            put(0, &response);
                            put_int(fd, &response);
                            send(socket_handle, response.buffer, 9, 0);
                            free(response.buffer);
                            break;
                        case 1:
                            fd = deserialize_int(&buffer[1]);
                            length = lseek(fd, 0, SEEK_END);
                            lseek(fd, 0, 0);
                            file = malloc(length);
                            read(fd, file, length);
                            init_buf(5+length, &response);
                            put_int(1+length, &response);
                            put(1, &response);
                            put_bytes(file, length, &response);
                            send(socket_handle, response.buffer, 5+length, 0);
                            free(response.buffer);
                            free(file);
                            break;
                        case 2:
                            fd = deserialize_int(&buffer[1]);
                            length = write(fd, &buffer[5], size - 5);
                            init_buf(9, &response);
                            put_int(5, &response);
                            put(2, &response);
                            put_int(length, &response);
                            send(socket_handle, response.buffer, 9, 0);
                            free(response.buffer);
                            break;
                        case 3:
                            printf("Not implemnted\n");
                            break;
                        case 4:
                            fd = deserialize_int(&buffer[1]);
                            length = close(fd);
                            init_buf(9, &response);
                            put_int(5, &response);
                            put(4, &response);
                            put_int(length, &response);
                            send(socket_handle, response.buffer, 9, 0);
                            free(response.buffer);
                            break;
                    }
                }
                exit(EXIT_SUCCESS);
            default:
                close(socket_conn);
                continue;
        }
    }
}
