MAKE_DIR = $(shell pwd)
SRC_DIR = $(MAKE_DIR)/src
BIN_DIR = $(MAKE_DIR)/bin
P_THREAD = -pthread
DEBUG = -g

all: web_server client

web_server: $(SRC_DIR)/web_server.c $(SRC_DIR)/server_utilities.o $(SRC_DIR)/task_queue.o $(SRC_DIR)/thr_pool.o
	$(CC) $(DEBUG) $(P_THREAD) $^ -o $(BIN_DIR)/$@

client: $(SRC_DIR)/client.c $(SRC_DIR)/client_utilities.o
	$(CC) $(DEBUG) $(P_THREAD) $^ -o $(BIN_DIR)/$@

%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h
	$(COMPILE.c) $(DEBUG) $(P_THREAD) $(OUTPUT_OPTION) $<

clean:
	rm $(BIN_DIR)/*
	rm $(SRC_DIR)/*.o
