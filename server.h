#ifndef SERVER_H
#define SERVER_H

#define BUFFER_SIZE 8192

typedef struct {
    const char *method;
    const char *path;
    const char *auth_header;
    int client_socket;
    void *user_data;
    void *plugin;  // Agregar este campo
} RequestData;

#endif // SERVER_H

