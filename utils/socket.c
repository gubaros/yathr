/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#include "socket.h"
#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        log_error("Socket creation failed: %s", strerror(errno));
        return -1;
    }

    log_info("Socket created successfully");

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        log_error("setsockopt SO_REUSEADDR failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }

#ifdef __linux__
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        log_error("setsockopt SO_REUSEPORT failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }
#endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        log_error("Bind failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }

    log_info("Socket bound to port %d", port);

    if (listen(server_fd, 1000) < 0) {
        log_error("Listen failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }

    log_info("Server listening on port %d", port);

    if (set_nonblocking(server_fd) == -1) {
        log_error("fcntl failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }

    return server_fd;
}
