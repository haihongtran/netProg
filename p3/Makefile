MAKE_DIR = $(shell pwd)
SRC_DIR = $(MAKE_DIR)/src
BIN_DIR = $(MAKE_DIR)/bin
P_THREAD = -pthread

all: test

test: $(SRC_DIR)/test.c $(SRC_DIR)/task_queue.o $(SRC_DIR)/thr_pool.o
	$(CC) $(P_THREAD) $^ -o $(BIN_DIR)/$@

%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h
	$(COMPILE.c) $(P_THREAD) $(OUTPUT_OPTION) $<

clean:
	rm $(BIN_DIR)/*
	rm $(SRC_DIR)/*.o
