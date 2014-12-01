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
#include <sys/stat.h>
#include "../serialize/serialize.h"

int main(int argc, char* argv[]) {
    struct sockaddr_in socket_info;
    char hostname[257];
    struct hostent* host;
    int socket_handle;
    int port = 0;
    int c;
    char* mount = NULL;

    //get args
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

    //make sure we got all the required args
    if (port == 0 || mount == NULL) {
        printf("Not enough args\n");
        exit(EXIT_FAILURE);
    }
    printf("Starting server...\n");
    printf("port: %d\n", port);
    printf("mount: %s\n", mount);

    //create the mount dir if it doesn't already exist
    mkdir(mount, S_IRWXU | S_IRWXG | S_IRWXO);

    bzero(&socket_info, sizeof(struct sockaddr_in));

    gethostname(hostname, 256);
    if ((host = gethostbyname(hostname)) == NULL) {
        printf("Error getting hostname\n");
        exit(EXIT_FAILURE);
    }

    //create the socket
    if ((socket_handle = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        close(socket_handle);
        exit(EXIT_FAILURE);
    }

    socket_info.sin_family = AF_INET;
    socket_info.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_info.sin_port = htons(port);

    //bind the socket to an address
    if (bind(socket_handle, (struct sockaddr*)&socket_info, sizeof(struct sockaddr_in)) < 0) {
        close(socket_handle);
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("Bound socket\n");

    //mark the socket as a listening socket
    listen(socket_handle, 1);
    printf("Listening on socket\n");

    int socket_conn;
    while (1) {
        //wait for connections
        if ((socket_conn = accept(socket_handle, NULL, NULL)) < 0) {
            close(socket_handle);
            if (errno == EINTR)
                continue;
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Accepted connection: %d\n", socket_conn);
        //when one is found, fork a process for it
        switch(fork()) {
            case -1:
                //shut down on failure
                perror("fork");
                close(socket_handle);
                close(socket_conn);
                exit(EXIT_FAILURE);
            case 0:
                while (1) {
                    //get the first 4 bytes (size of message)
                    char buf[4];
                    int rc = recv(socket_conn, buf, sizeof(buf), 0);
                    if (rc < 4)
                        continue;
                    int size = deserialize_int(buf);
                    printf("Size of message: %d\n", size);
                    char* buffer = malloc(size);
                    rc = recv(socket_conn, buffer, size, 0);
                    if (rc < size)
                        exit(EXIT_FAILURE);
                    int id = buffer[0];
                    printf("Message id: %d\n", id);
                    char* filename;
                    int fd;
                    int length;
                    int sent;
                    struct stat stat_buf;
                    void* file;
                    byte_buffer response;
                    //switch on which message is recieved
                    switch (id) {
                        case 0:
                            //get full filename
                            filename = malloc(strlen(mount) + size + 1);
                            strcpy(filename, mount);
                            strcat(filename, "/");
                            strncat(filename, &buffer[1], size - 1);
                            printf("Opening file: %s\n", filename);
                            //open file with full permissions (and be sure to truncate)
                            fd = open(filename, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
                            //create the buffer to write back to the client
                            init_buf(9, &response);
                            put_int(5, &response);
                            put(0, &response);
                            put_int(fd, &response);
                            printf("Sending fd: %d\n", fd);
                            //write data back to client
                            sent = send(socket_conn, response.buffer, 9, 0);
                            printf("Sent %d bytes\n", sent);
                            free(response.buffer);
                            break;
                        case 1:
                            printf("Got read message\n");
                            //get file descriptor
                            fd = deserialize_int(&buffer[1]);
                            //get the length of the file by lseeking to the end
                            length = lseek(fd, 0, SEEK_END);
                            lseek(fd, 0, 0);
                            //create a buffer of this length and read everything into it
                            file = malloc(length);
                            printf ("Reading from file: %d\n", fd);
                            read(fd, file, length);
                            printf("Read: %.*s\n", length, file);
                            //create the response buffer and write it to the socket
                            init_buf(5+length, &response);
                            put_int(1+length, &response);
                            put(1, &response);
                            put_bytes(file, length, &response);
                            printf("Sending bytes: %d\n", 5+length);
                            sent = send(socket_conn, response.buffer, 5+length, 0);
                            printf("Sent %d bytes\n", sent);
                            free(response.buffer);
                            free(file);
                            break;
                        case 2:
                            printf ("Got write message\n");
                            //get file descriptor and truncate the file
                            fd = deserialize_int(&buffer[1]);
                            if (ftruncate(fd, 0) == -1) {
                                perror("Error: truncate");
                                break;
                            }
                            lseek(fd, 0, 0);
                            //write the data to the file
                            printf ("Writing to file: %d\n", fd);
                            if ((length = write(fd, &buffer[5], size - 5)) < 0) {
                                perror("Error: Write");
                            }
                            printf("Wrote %.*s\n", length, &buffer[5]);
                            //return response through socket
                            init_buf(9, &response);
                            put_int(5, &response);
                            put(2, &response);
                            put_int(length, &response);
                            printf("Sending length: %d\n", length);
                            sent = send(socket_conn, response.buffer, 9, 0);
                            printf("Sent %d bytes\n", sent);
                            free(response.buffer);
                            break;
                        case 3:
                            //get file descriptor
                            fd = deserialize_int(&buffer[1]);
                            //stat file
                            fstat(fd, &stat_buf);
                            //put stat data into buffer and send through socket
                            init_buf(21, &response);
                            put_int(17, &response);
                            put(3, &response);
                            put_int(stat_buf.st_size, &response);
                            put_int(stat_buf.st_ctim.tv_sec, &response);
                            put_int(stat_buf.st_atim.tv_sec, &response);
                            put_int(stat_buf.st_mtim.tv_sec, &response);
                            printf("Sending size: %d, ctime: %d, atime: %d, mtime: %d\n",
                                    deserialize_int(&response.buffer[5]),
                                    deserialize_int(&response.buffer[9]),
                                    deserialize_int(&response.buffer[13]),
                                    deserialize_int(&response.buffer[17]));
                            send(socket_conn, response.buffer, 21, 0);
                            free(response.buffer);
                            break;
                        case 4:
                            //get file descriptor
                            fd = deserialize_int(&buffer[1]);
                            //close the file and send the response
                            length = close(fd);
                            init_buf(9, &response);
                            put_int(5, &response);
                            put(4, &response);
                            put_int(length, &response);
                            send(socket_conn, response.buffer, 9, 0);
                            free(response.buffer);
                            break;
                        case 5:
                            //close the socket and kill the process
                            printf("Closing connection\n");
                            close(socket_conn);
                            exit(EXIT_SUCCESS);
                    }
                }
            default:
                continue;
        }
    }
}
