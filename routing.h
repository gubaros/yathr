/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#ifndef ROUTING_H
#define ROUTING_H

const char *find_redirect(const char *key);
int add_redirect(const char *key, const char *url);
void init_routing(void);
void cleanup_routing(void);

#endif // ROUTING_H
