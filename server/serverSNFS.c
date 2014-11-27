#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main() {
    struct sockaddr_in socket_info;
    char hostname[257];
    struct hostent* host;
    int socket_handle;
    int port = 5000;

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
                //do stuff
                exit(EXIT_SUCCESS);
            default:
                close(socket_conn);
                continue;
        }
    }
}
