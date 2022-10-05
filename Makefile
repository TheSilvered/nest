CC=gcc
CFLAGS=-Wall -g
CLINKS=-lm -ldl
EXE_NAME=nest

SRC_DIR=nest
EXE_DIR=unix_executable

SRCS = $(shell find $(SRC_DIR) -name *.c -or -name *.cpp)
TARGET = $(EXE_DIR)/$(EXE_NAME)

.PHONY: clean libs all optimized debug

$(TARGET): $(SRCS)
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -o $@

clean:
	rm -fr $(OBJ_DIR)/*.o
	rm -fr $(TARGET)

optimized:
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $(TARGET)

debug:
	mkdir -p $(EXE_DIR)
	$(CC) -D_DEBUG $(CFLAGS) $(SRCS) $(CLINKS) -o $(TARGET)
