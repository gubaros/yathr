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
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>

void handle_request(int client_socket, const char *method, const char *path, const char *auth_header) {
    RequestData request_data = {method, path, auth_header, client_socket, NULL};

    execute_plugins(PRE_ROUTING, &request_data);

    // Skip leading '/' for routing lookup (optimized: avoid strlen)
    const char *key = path;
    if (path && *path == '/') {
        key = path + 1;
        // Handle empty path case
        if (*key == '\0') {
            key = NULL;
        }
    }
    /* Fast path: send the pre-built response stored in the routing table. */
    size_t response_len = 0;
    const char *cached_response = key ? find_redirect_response(key, &response_len) : NULL;

    if (cached_response) {
        send(client_socket, cached_response, response_len, 0);
        log_info("Redirected %s to %s", path, find_redirect(key));
    } else {
        static const char not_found[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
        send(client_socket, not_found, sizeof(not_found) - 1, 0);
        log_warning("Path not found: %s", path);
    }

    execute_plugins(POST_ROUTING, &request_data);
    close(client_socket);
}
