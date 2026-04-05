/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#ifndef ROUTING_H
#define ROUTING_H

#include <stddef.h>

const char *find_redirect(const char *key);
const char *find_redirect_response(const char *key, size_t *response_len);
int add_redirect(const char *key, const char *url);
void init_routing(void);
void cleanup_routing(void);

#endif // ROUTING_H
