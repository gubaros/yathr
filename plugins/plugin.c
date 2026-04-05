/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#include "plugin.h"
#include <stdio.h>
#include <string.h>

#define MAX_PLUGINS 128

typedef struct {
    const char *name;
    PluginType type;
    PluginFunction execute;
} Plugin;

static Plugin plugins[MAX_PLUGINS];
static int plugin_count = 0;

void register_plugin(const char *name, PluginType type, PluginFunction execute) {
    if (plugin_count < MAX_PLUGINS) {
        plugins[plugin_count].name = name;
        plugins[plugin_count].type = type;
        plugins[plugin_count].execute = execute;
        plugin_count++;
    } else {
        fprintf(stderr, "Max plugins reached\n");
    }
}

void execute_plugins(PluginType type, RequestData *request_data) {
    for (int i = 0; i < plugin_count; i++) {
        if (plugins[i].type == type) {
            request_data->plugin = &plugins[i];
            plugins[i].execute(request_data);
        }
    }
}
