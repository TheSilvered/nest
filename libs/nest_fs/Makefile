CC=g++
CFLAGS=-Wall -g -shared -fPIC -rdynamic -std=c++2a
CLINKS=-lm -ldl
EXE_NAME=stdfs.so

SRC_DIR=../../src
EXE_DIR=../../unix_release
DBG_DIR=../../unix_debug

SRCS := $(filter-out nest.c argv_parser.c, $(wildcard $(SRC_DIR)/*.c))
DBG_TARGET := $(DBG_DIR)/$(EXE_NAME)
EXE_TARGET := $(EXE_DIR)/$(EXE_NAME)

.PHONY: debug

$(EXE_TARGET): $(SRCS)
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $@

debug:
	mkdir -p $(DBG_DIR)
	# echo $(SRCS)
	$(CC) -D_DEBUG $(CFLAGS) $(SRCS) $(CLINKS) -o $(DBG_TARGET)
