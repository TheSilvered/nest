CC=g++
CFLAGS=-Wall -shared -fPIC -rdynamic -std=c++2a
CLINKS=-lm -ldl
EXE_NAME=stdio.so

SRC_DIR=../../src
EXE_DIR=../../unix_release
DBG_DIR=../../unix_debug

LIB_SRCS=nest_io/nest_io.cpp
SRCS := $(filter-out nest.c argv_parser.c, $(wildcard $(SRC_DIR)/*.c))
DBG_TARGET := $(DBG_DIR)/$(EXE_NAME)
EXE_x64_TARGET := $(EXE_DIR)/x64/$(EXE_NAME)
EXE_x86_TARGET := $(EXE_DIR)/x86/$(EXE_NAME)

.PHONY: debug x86

$(EXE_x64_TARGET): $(SRCS)
	mkdir -p $(EXE_DIR)/x64
	$(CC) $(CFLAGS) $(SRCS) $(LIB_SRCS) $(CLINKS) -O3 -o $@

x86:
	mkdir -p $(EXE_DIR)/x86
	$(CC) $(CFLAGS) $(SRCS) $(LIB_SRCS) $(CLINKS) -O3 -m32 -o $(EXE_x86_TARGET)

debug:
	mkdir -p $(DBG_DIR)
	$(CC) -D_DEBUG -g $(CFLAGS) $(SRCS) $(LIB_SRCS) $(CLINKS) -o $(DBG_TARGET)
