/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#ifndef HTTP_H
#define HTTP_H

void handle_request(int client_socket, const char *method, const char *path, const char *auth_header);

#endif // HTTP_H
