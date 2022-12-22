CC=gcc
CFLAGS=-Wall
CLINKS=-lm -ldl
EXE_NAME=nest

SRC_DIR=src
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
	$(CC) -D_DEBUG -g $(CFLAGS) $(SRCS) $(CLINKS) -o $(DBG_TARGET)

all:
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $@
	cd libs/nest_co;     make
	cd libs/nest_err;    make
	cd libs/nest_fs;     make
	cd libs/nest_io;     make
	cd libs/nest_itutil; make
	cd libs/nest_math;   make
	cd libs/nest_rand;   make
	cd libs/nest_sequtil;make
	cd libs/nest_sutil;  make
	cd libs/nest_sys;    make
	cd libs/nest_time;   make

all-debug:
	mkdir -p $(DBG_DIR)
	$(CC) -D_DEBUG $(CFLAGS) $(SRCS) $(CLINKS) -o $(DBG_TARGET)
	cd libs/nest_co;     make debug
	cd libs/nest_err;    make debug
	cd libs/nest_fs;     make debug
	cd libs/nest_io;     make debug
	cd libs/nest_itutil; make debug
	cd libs/nest_math;   make debug
	cd libs/nest_rand;   make debug
	cd libs/nest_sequtil;make debug
	cd libs/nest_sutil;  make debug
	cd libs/nest_sys;    make debug
	cd libs/nest_time;   make debug
