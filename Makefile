CC=gcc
CFLAGS= -Wall -g
EXE_NAME=nest

SRC_DIR=nest
EXE_DIR=unix_executable

SRCS = $(shell find $(SRC_DIR) -name *.c -or -name *.cpp)
TARGET = $(EXE_DIR)/$(EXE_NAME)

.PHONY: clean libs all optimized

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -lm -o $@

clean:
	rm -fr $(OBJ_DIR)/*.o
	rm -fr $(TARGET)

optimized:
	$(CC) $(CFLAGS) $(SRCS) -lm -O3 -o $(TARGET)
