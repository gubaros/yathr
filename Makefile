CC = gcc
CFLAGS = -Wall -O2 -march=native -flto
LDFLAGS = -lcdb

# Variables de entorno para las rutas de zlog
ZLOG_INCLUDE_PATH ?= /usr/local/include
ZLOG_LIB_PATH ?= /usr/local/lib

CFLAGS += -I$(ZLOG_INCLUDE_PATH)
LDFLAGS += -L$(ZLOG_LIB_PATH) -lzlog

PLUGIN_DIR = plugins
UTILS_DIR = utils
PLUGINS = $(PLUGIN_DIR)/plugin.c $(PLUGIN_DIR)/pre_routing_plugin.c $(PLUGIN_DIR)/post_routing_plugin.c

all: http_server create

http_server: server.o platform.o $(UTILS_DIR)/logs.o $(PLUGINS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

create: create.o
	$(CC) $(CFLAGS) -o $@ create.o $(LDFLAGS)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

platform.o: platform.c
	$(CC) $(CFLAGS) -c platform.c

$(UTILS_DIR)/logs.o: $(UTILS_DIR)/logs.c
	$(CC) $(CFLAGS) -c $(UTILS_DIR)/logs.c -o $(UTILS_DIR)/logs.o

create.o: create.c
	$(CC) $(CFLAGS) -c create.c

clean:
	rm -f http_server create *.o $(PLUGIN_DIR)/*.o $(UTILS_DIR)/*.o

