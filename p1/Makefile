MAKE_DIR = $(shell pwd)
SRC_DIR = $(MAKE_DIR)/src
BIN_DIR = $(MAKE_DIR)/bin

all: server client

server: $(SRC_DIR)/server.o
	$(CC) $^ -o $(BIN_DIR)/$@

client: $(SRC_DIR)/client.o
	$(CC) $^ -o $(BIN_DIR)/$@

%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h
	$(COMPILE.c) $(OUTPUT_OPTION) $<

clean:
	rm $(SRC_DIR)/*.o
	rm $(BIN_DIR)/*
