/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#include "http.h"
#include "routing.h"
#include "server.h"
#include "plugins/plugin.h"
#include "utils/logs.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

void handle_request(int client_socket, const char *method, const char *path, const char *auth_header) {
    RequestData request_data = {method, path, auth_header, client_socket, NULL};

    execute_plugins(PRE_ROUTING, &request_data);

    const char *redirect_url = find_redirect(path + 1);
    if (redirect_url) {
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "HTTP/1.1 302 Found\nLocation: %s\nContent-Length: 0\n\n", redirect_url);
        send(client_socket, response, strlen(response), 0);
        log_info("Redirected %s to %s", path, redirect_url);
    } else {
        const char *response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 9\n\nNot Found";
        send(client_socket, response, strlen(response), 0);
        log_warning("Path not found: %s", path);
    }

    execute_plugins(POST_ROUTING, &request_data);
    close(client_socket);
}
