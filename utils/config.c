/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#include "config.h"
#include "logs.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

int read_port_from_config(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_error("fopen %s failed: %s", filename, strerror(errno));
        return -1;
    }

    char buffer[128];
    int port = -1;

    while (fgets(buffer, sizeof(buffer), file)) {
        if (sscanf(buffer, "SERVER_PORT=%d", &port) == 1) {
            break;
        }
    }

    fclose(file);
    return port;
}
