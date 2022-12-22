CC=g++
CFLAGS=-Wall -shared -fPIC -rdynamic -std=c++2a
CLINKS=-lm -ldl
EXE_NAME=stditutil.so

SRC_DIR=../../src
EXE_DIR=../../unix_release
DBG_DIR=../../unix_debug

LIB_SRCS=nest_itutil/nest_itutil.cpp nest_itutil/itutil_functions.cpp
SRCS := $(filter-out nest.c argv_parser.c, $(wildcard $(SRC_DIR)/*.c))
DBG_TARGET := $(DBG_DIR)/$(EXE_NAME)
EXE_TARGET := $(EXE_DIR)/$(EXE_NAME)

.PHONY: debug

$(EXE_TARGET): $(SRCS)
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(LIB_SRCS) $(CLINKS) -O3 -o $@

debug:
	mkdir -p $(DBG_DIR)
	$(CC) -D_DEBUG -g $(CFLAGS) $(SRCS) $(LIB_SRCS) $(CLINKS) -o $(DBG_TARGET)
