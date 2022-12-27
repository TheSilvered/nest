CC=gcc
CFLAGS=-I../../../include -Wall
CLINKS=-lm -ldl
EXE_NAME=nest

SRC_DIR=../../../src
EXE_DIR=../linux_release
EXE_x64_DIR:=$(EXE_DIR)/x64
EXE_x86_DIR=$(EXE_DIR)/x86
DBG_DIR=../linux_debug

SRCS := $(wildcard $(SRC_DIR)/*.c)
DBG_TARGET := $(DBG_DIR)/$(EXE_NAME)
EXE_x64_TARGET := $(EXE_x64_DIR)/$(EXE_NAME)
EXE_x86_TARGET := $(EXE_x86_DIR)/$(EXE_NAME)

.PHONY: clean all debug all-debug x86 all-x86

$(EXE_x64_TARGET): $(SRCS)
	mkdir -p $(EXE_x64_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $@

x86:
	mkdir -p $(EXE_x86_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -m32 -o $(EXE_x86_TARGET)

clean:
	rm -fr $(EXE_DIR)
	rm -fr $(DBG_DIR)

debug:
	mkdir -p $(DBG_DIR)
	$(CC) -D_DEBUG -g $(CFLAGS) $(SRCS) $(CLINKS) -o $(DBG_TARGET)

all:
	mkdir -p $(EXE_x64_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $(EXE_x64_TARGET)
	make -f nest_co.mk
	make -f nest_err.mk
	make -f nest_fs.mk
	make -f nest_io.mk
	make -f nest_itutil.mk
	make -f nest_math.mk
	make -f nest_rand.mk
	make -f nest_sequtil.mk
	make -f nest_sutil.mk
	make -f nest_sys.mk
	make -f nest_time.mk

all-x86:
	mkdir -p $(EXE_x86_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -m32 -o $(EXE_x86_TARGET)
	make -f nest_co.mk      x86
	make -f nest_err.mk     x86
	make -f nest_fs.mk      x86
	make -f nest_io.mk      x86
	make -f nest_itutil.mk  x86
	make -f nest_math.mk    x86
	make -f nest_rand.mk    x86
	make -f nest_sequtil.mk x86
	make -f nest_sutil.mk   x86
	make -f nest_sys.mk     x86
	make -f nest_time.mk    x86

all-debug:
	mkdir -p $(DBG_DIR)
	$(CC) -D_DEBUG $(CFLAGS) $(SRCS) $(CLINKS) -o $(DBG_TARGET)
	make -f nest_co.mk      debug
	make -f nest_err.mk     debug
	make -f nest_fs.mk      debug
	make -f nest_io.mk      debug
	make -f nest_itutil.mk  debug
	make -f nest_math.mk    debug
	make -f nest_rand.mk    debug
	make -f nest_sequtil.mk debug
	make -f nest_sutil.mk   debug
	make -f nest_sys.mk     debug
	make -f nest_time.mk    debug
