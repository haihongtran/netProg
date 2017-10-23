MAKE_DIR = $(shell pwd)
SRC_DIR = $(MAKE_DIR)/src
BIN_DIR = $(MAKE_DIR)/bin

all: server client
	rm $(SRC_DIR)/*.o

server: $(SRC_DIR)/server.c $(SRC_DIR)/server_utilities.o $(SRC_DIR)/common_utilities.o
	$(CC) $^ -o $(BIN_DIR)/$@

client: $(SRC_DIR)/client.c $(SRC_DIR)/client_utilities.o $(SRC_DIR)/common_utilities.o
	$(CC) $^ -o $(BIN_DIR)/$@

%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h
	$(COMPILE.c) $(OUTPUT_OPTION) $<

clean:
	rm $(BIN_DIR)/*
