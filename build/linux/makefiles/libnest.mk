CC = gcc
CFLAGS = -I../../../include -Wall -shared -fPIC -rdynamic
DBG_FLAGS = -D_DEBUG -g -O0
EXE_NAME = libnest.so

SRC_DIR = ../../../src
x64_DIR = ../linux_release/x64
x86_DIR = ../linux_release/x86
DBG_DIR = ../linux_debug

CLINKS = -lm -ldl

SRCS := $(filter-out $(SRC_DIR)/nest.c, $(wildcard $(SRC_DIR)/*.c))
DBG_TARGET := $(DBG_DIR)/$(EXE_NAME)
x64_TARGET := $(x64_DIR)/$(EXE_NAME)
x86_TARGET := $(x86_DIR)/$(EXE_NAME)

.PHONY: debug x86

$(x64_TARGET): $(SRCS)
	mkdir -p $(x64_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $(x64_TARGET)

x86: $(SRCS)
	mkdir -p $(x86_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -m32 -o $(x86_TARGET)

debug: $(SRCS)
	mkdir -p $(DBG_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) $(DBG_FLAGS) -o $(DBG_TARGET)
