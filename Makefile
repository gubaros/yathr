CC = gcc
CFLAGS = -Wall -O2 -march=native -flto
LDFLAGS =

# Variables de entorno para las rutas de zlog
ZLOG_INCLUDE_PATH ?= /opt/homebrew/include
ZLOG_LIB_PATH ?= /opt/homebrew/lib

CFLAGS += -I$(ZLOG_INCLUDE_PATH)
LDFLAGS += -L$(ZLOG_LIB_PATH) -lzlog

PLUGIN_DIR = plugins
UTILS_DIR = utils
PLUGINS = $(PLUGIN_DIR)/plugin.c $(PLUGIN_DIR)/pre_routing_plugin.c $(PLUGIN_DIR)/post_routing_plugin.c

all: http_server

http_server: server.o platform.o routing.o http.o $(UTILS_DIR)/logs.o $(UTILS_DIR)/config.o $(UTILS_DIR)/socket.o $(PLUGINS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

platform.o: platform.c
	$(CC) $(CFLAGS) -c platform.c

routing.o: routing.c
	$(CC) $(CFLAGS) -c routing.c

http.o: http.c
	$(CC) $(CFLAGS) -c http.c

$(UTILS_DIR)/logs.o: $(UTILS_DIR)/logs.c
	$(CC) $(CFLAGS) -c $(UTILS_DIR)/logs.c -o $(UTILS_DIR)/logs.o

$(UTILS_DIR)/config.o: $(UTILS_DIR)/config.c
	$(CC) $(CFLAGS) -c $(UTILS_DIR)/config.c -o $(UTILS_DIR)/config.o

$(UTILS_DIR)/socket.o: $(UTILS_DIR)/socket.c
	$(CC) $(CFLAGS) -c $(UTILS_DIR)/socket.c -o $(UTILS_DIR)/socket.o

clean:
	rm -f http_server *.o $(PLUGIN_DIR)/*.o $(UTILS_DIR)/*.o my_log.*
	rm -f tests/test_routing tests/test_config

TESTS_DIR = tests
UNITY_SRC = $(TESTS_DIR)/unity/unity.c

# Unit tests
$(TESTS_DIR)/test_routing: $(TESTS_DIR)/test_routing.c $(UNITY_SRC) routing.c
	$(CC) $(CFLAGS) -I$(TESTS_DIR) -I. -o $@ $^

$(TESTS_DIR)/test_config: $(TESTS_DIR)/test_config.c $(UNITY_SRC) $(TESTS_DIR)/logs_stub.c $(UTILS_DIR)/config.c
	$(CC) $(CFLAGS) -I$(TESTS_DIR) -I. -o $@ $^

.PHONY: test
test: http_server $(TESTS_DIR)/test_routing $(TESTS_DIR)/test_config
	@echo "=== Unit Tests ==="
	./$(TESTS_DIR)/test_routing
	./$(TESTS_DIR)/test_config
	@echo ""
	@echo "=== Integration Tests ==="
	bash $(TESTS_DIR)/integration.sh

