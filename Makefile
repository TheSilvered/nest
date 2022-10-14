CC=gcc
CFLAGS=-Wall -g
CLINKS=-lm -ldl
EXE_NAME=nest

SRC_DIR=nest
EXE_DIR=unix_release
DBG_DIR=unix_debug

SRCS := $(wildcard $(SRC_DIR)/*.c)
DBG_TARGET := $(DBG_DIR)/$(EXE_NAME)
EXE_TARGET := $(EXE_DIR)/$(EXE_NAME)

.PHONY: clean all debug all-debug

$(EXE_TARGET): $(SRCS)
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $@

clean:
	rm -fr $(EXE_DIR)
	rm -fr $(DBG_DIR)

debug:
	mkdir -p $(DBG_DIR)
	$(CC) -D_DEBUG $(CFLAGS) $(SRCS) $(CLINKS) -o $(DBG_TARGET)

all:
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $@
	cd nest_libs/nest_fs
	make

all-debug:
	mkdir -p $(DBG_DIR)
	$(CC) -D_DEBUG $(CFLAGS) $(SRCS) $(CLINKS) -o $(DBG_TARGET)
	cd nest_libs/nest_fs
	make debug
