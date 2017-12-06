MAKE_DIR = $(shell pwd)
SRC_DIR = $(MAKE_DIR)/src
BIN_DIR = $(MAKE_DIR)/bin
P_THREAD = -pthread
DEBUG = -g

all: super child

super: $(SRC_DIR)/super.c $(SRC_DIR)/super_lib.o $(SRC_DIR)/common.o
	$(CC) $(DEBUG) $^ -o $(BIN_DIR)/$@

child: $(SRC_DIR)/child.c $(SRC_DIR)/child_lib.o $(SRC_DIR)/common.o
	$(CC) $(DEBUG) $(P_THREAD) $^ -o $(BIN_DIR)/$@

%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h
	$(COMPILE.c) $(DEBUG) $(P_THREAD) $(OUTPUT_OPTION) $<

clean:
	rm $(BIN_DIR)/*
	rm $(SRC_DIR)/*.o
