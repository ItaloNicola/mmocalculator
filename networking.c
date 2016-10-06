#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "networking.h"

#define SERVER_ADDRESS "127.0.0.1"
#define PORT_NUMBER 5358

int socket_fd;

int connectToServer() {

    // create TCP (NON-BLOCKING) socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) return -1;

    // load host by address and verify if it exists
    struct hostent *server = gethostbyname(SERVER_ADDRESS);
    if (server == NULL) return -1;

    // server address struct initialization
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(PORT_NUMBER);

    // connect to server
    int er = connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0;
    if (er) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return -1;
    }

    // set file descriptor to non-blocking mode
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);

    return 0;
}

int writeToServer(char *message, int size) {
    int n = write(socket_fd, message, size);
    if (n < 0) return -1;

    return n;
}

// DEPRECATED
int readFromServer(char *message, int size) {
    int n = read(socket_fd, message, size - 1);
    if (n < 0) return -1;

    return n;
}

int pullUpdates255(void *message) {
    char buffer[256];

    memset(buffer, 0, 256);

    int n = read(socket_fd, buffer, 256);
    if (n > 0) {
        strcpy(message, buffer);
    }

    return 1;
}
