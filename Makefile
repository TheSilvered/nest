CC=gcc
CFLAGS= -Wall -g
EXE_NAME=nest

SRC_DIR=nest
OBJ_DIR=unix_build
EXE_DIR=unix_executable

SRCS = $(shell find $(SRC_DIR) -name *.c -or -name *.cpp)
OBJS = $(subst $(SRC_DIR)/, $(OBJ_DIR)/, $(addsuffix .o,$(basename $(SRCS))))
TARGET = $(EXE_DIR)/$(EXE_NAME)

.PHONY: clean

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -fr $(OBJ_DIR)/*.o
