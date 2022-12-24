CC=gcc
CFLAGS=-Wall
CLINKS=-lm -ldl
EXE_NAME=nest

SRC_DIR=src
EXE_DIR=unix_release
DBG_DIR=unix_debug

SRCS := $(wildcard $(SRC_DIR)/*.c)
DBG_TARGET := $(DBG_DIR)/$(EXE_NAME)
EXE_x64_TARGET := $(EXE_DIR)/x64/$(EXE_NAME)
EXE_x86_TARGET := $(EXE_DIR)/x86/$(EXE_NAME)

.PHONY: clean all debug all-debug x86 all-x86

$(EXE_x64_TARGET): $(SRCS)
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $@

x86:
	mkdir -p $(EXE_DIR)/x86
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -m32 -o $(EXE_x86_TARGET)

clean:
	rm -fr $(EXE_DIR)
	rm -fr $(DBG_DIR)

debug:
	mkdir -p $(DBG_DIR)
	$(CC) -D_DEBUG -g $(CFLAGS) $(SRCS) $(CLINKS) -o $(DBG_TARGET)

all:
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $(EXE_x64_TARGET)
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

all-x86:
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -m32 -o $(EXE_x86_TARGET)
	cd libs/nest_co;     make x86
	cd libs/nest_err;    make x86
	cd libs/nest_fs;     make x86
	cd libs/nest_io;     make x86
	cd libs/nest_itutil; make x86
	cd libs/nest_math;   make x86
	cd libs/nest_rand;   make x86
	cd libs/nest_sequtil;make x86
	cd libs/nest_sutil;  make x86
	cd libs/nest_sys;    make x86
	cd libs/nest_time;   make x86

all-debug:
	mkdir -p $(DBG_DIR)/x64
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
