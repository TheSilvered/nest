CC=g++
CFLAGS=-I../../../include -Wall -shared -fPIC -rdynamic -std=c++2a
CLINKS=-lm -ldl
DBG_FLAGS=-D_DEBUG -g -O0
EXE_NAME=stdjson.so

SRC_DIR=../../../src
EXE_x64_DIR=../linux_release/x64
EXE_x86_DIR=../linux_release/x86
DBG_DIR=../linux_debug

LIB_SRCS=../../../libs/nest_json/nest_json.cpp
SRCS := $(filter-out nest.c argv_parser.c, $(wildcard $(SRC_DIR)/*.c))
DBG_TARGET := $(DBG_DIR)/$(EXE_NAME)
EXE_x64_TARGET := $(EXE_x64_DIR)/$(EXE_NAME)
EXE_x86_TARGET := $(EXE_x86_DIR)/$(EXE_NAME)

.PHONY: debug x86

$(EXE_x64_TARGET): $(SRCS) $(LIB_SRCS)
	mkdir -p $(EXE_x64_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(LIB_SRCS) $(CLINKS) -O3 -o $@

x86:
	mkdir -p $(EXE_x86_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(LIB_SRCS) $(CLINKS) -O3 -m32 -o $(EXE_x86_TARGET)

debug:
	mkdir -p $(DBG_DIR)
	$(CC) $(DBG_FLAGS) $(CFLAGS) $(SRCS) $(LIB_SRCS) $(CLINKS) -o $(DBG_TARGET)
