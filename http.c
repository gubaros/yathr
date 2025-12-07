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
    const char *redirect_url = key ? find_redirect(key) : NULL;
    
    if (redirect_url) {
        // Optimized response formatting: avoid snprintf overhead
        static const char header[] = "HTTP/1.1 302 Found\r\nLocation: ";
        static const char footer[] = "\r\nContent-Length: 0\r\n\r\n";
        size_t url_len = strlen(redirect_url);
        size_t header_len = sizeof(header) - 1;
        size_t footer_len = sizeof(footer) - 1;
        size_t total_len = header_len + url_len + footer_len;
        
        if (total_len < BUFFER_SIZE) {
            char response[BUFFER_SIZE];
            char *p = response;
            memcpy(p, header, header_len);
            p += header_len;
            memcpy(p, redirect_url, url_len);
            p += url_len;
            memcpy(p, footer, footer_len);
            p += footer_len;
            send(client_socket, response, total_len, 0);
        } else {
            // Fallback for very long URLs
            char response[BUFFER_SIZE];
            snprintf(response, sizeof(response), "HTTP/1.1 302 Found\r\nLocation: %s\r\nContent-Length: 0\r\n\r\n", redirect_url);
            send(client_socket, response, strlen(response), 0);
        }
        log_info("Redirected %s to %s", path, redirect_url);
    } else {
        static const char not_found[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
        send(client_socket, not_found, sizeof(not_found) - 1, 0);
        log_warning("Path not found: %s", path);
    }

    execute_plugins(POST_ROUTING, &request_data);
    close(client_socket);
}
