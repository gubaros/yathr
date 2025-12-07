/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#ifndef SOCKET_H
#define SOCKET_H

int set_nonblocking(int fd);
int create_server_socket(int port);

#endif // SOCKET_H
