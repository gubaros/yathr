/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#ifdef __linux__
#include <sys/epoll.h>
#else
#include <sys/event.h>
#endif
#include "server.h"
#include "platform.h"
#include "utils/logs.h"
#include "utils/config.h"
#include "utils/socket.h"

#define MAX_EVENTS 1024

int main() {
    int server_fd = -1, loop_fd = -1, nev;
#ifdef __linux__
    struct epoll_event events[MAX_EVENTS];
#else
    struct kevent events[MAX_EVENTS];
#endif
    char buffer[BUFFER_SIZE];

    init_logs();

    int port = read_port_from_config("config.txt");
    if (port == -1) {
        log_error("Failed to read port from config");
        exit(EXIT_FAILURE);
    }

    server_fd = create_server_socket(port);
    if (server_fd == -1) {
        log_error("Failed to create server socket");
        exit(EXIT_FAILURE);
    }

    if ((loop_fd = create_event_loop()) == -1) {
        log_error("create_event_loop failed: %s", strerror(errno));
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (add_to_event_loop(loop_fd, server_fd) == -1) {
        log_error("add_to_event_loop failed: %s", strerror(errno));
        close(server_fd);
        close(loop_fd);
        exit(EXIT_FAILURE);
    }

    log_info("Event loop started");

    while (1) {
        nev = wait_for_events(loop_fd, events, MAX_EVENTS);
        if (nev < 0) {
            log_error("wait_for_events failed: %s", strerror(errno));
            close(server_fd);
            close(loop_fd);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nev; i++) {
            handle_event(loop_fd, &events[i], server_fd, buffer, BUFFER_SIZE);
        }
    }

    close(server_fd);
    close(loop_fd);
    return 0;
}
