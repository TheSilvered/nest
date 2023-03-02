CC = gcc
CFLAGS = -I../../../include -Wall
DBG_FLAGS = -D_DEBUG -g -O0
EXE_NAME = nest

SRC_DIR = ../../../src
EXE_DIR = ../linux_release
x64_DIR:= $(EXE_DIR)/x64
x86_DIR:= $(EXE_DIR)/x86
DBG_DIR = ../linux_debug

CLINKS = -lm -ldl -lnest
CLINK_DIR_DBG := -L$(DBG_DIR)
CLINK_DIR_x86 := -L$(x86_DIR)
CLINK_DIR_x64 := -L$(x64_DIR)
CLINKS_DBG := $(CLINK_DIR_DBG) $(CLINKS)
CLINKS_x86 := $(CLINK_DIR_x86) $(CLINKS)
CLINKS_x64 := $(CLINK_DIR_x64) $(CLINKS)

SRCS = $(SRC_DIR)/nest.c
DBG_TARGET := $(DBG_DIR)/$(EXE_NAME)
x64_TARGET := $(x64_DIR)/$(EXE_NAME)
x86_TARGET := $(x86_DIR)/$(EXE_NAME)
NEST_LIB_DBG := $(DBG_DIR)/libnest.so
NEST_LIB_x64 := $(x64_DIR)/libnest.so
NEST_LIB_x86 := $(x86_DIR)/libnest.so

.PHONY: debug x86 clean all all-debug all-x86

$(x64_TARGET): $(SRCS) $(NEST_LIB_x64)
	mkdir -p $(x64_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x64) -O3 -o $(x64_TARGET)

x86: $(x86_TARGET);
$(x86_TARGET): $(SRCS) $(NEST_LIB_x86)
	mkdir -p $(x86_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x86) -O3 -m32 -o $(x86_TARGET)

debug: $(DBG_TARGET);
$(DBG_TARGET): $(SRCS) $(NEST_LIB_DBG)
	mkdir -p $(DBG_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS_DBG) $(DBG_FLAGS) -o $(DBG_TARGET)

$(NEST_LIB_x64):
	make -f libnest.mk

$(NEST_LIB_x86):
	make -f libnest.mk x86

$(NEST_LIB_DBG):
	make -f libnest.mk debug

clean:
	rm -fr $(EXE_DIR)
	rm -fr $(DBG_DIR)

all:
	make -f libnest.mk
	make
	make -f nest_co.mk
	make -f nest_err.mk
	make -f nest_fs.mk
	make -f nest_io.mk
	make -f nest_itutil.mk
	make -f nest_json.mk
	make -f nest_math.mk
	make -f nest_rand.mk
	make -f nest_sequtil.mk
	make -f nest_sutil.mk
	make -f nest_sys.mk
	make -f nest_time.mk
	make -f nest_utf8.mk

all-x86:
	make -f libnest.mk      x86
	make                    x86
	make -f nest_co.mk      x86
	make -f nest_err.mk     x86
	make -f nest_fs.mk      x86
	make -f nest_io.mk      x86
	make -f nest_itutil.mk  x86
	make -f nest_json.mk    x86
	make -f nest_math.mk    x86
	make -f nest_rand.mk    x86
	make -f nest_sequtil.mk x86
	make -f nest_sutil.mk   x86
	make -f nest_sys.mk     x86
	make -f nest_time.mk    x86
	make -f nest_utf8.mk    x86

all-debug:
	make -f libnest.mk      debug
	make                    debug
	make -f nest_co.mk      debug
	make -f nest_err.mk     debug
	make -f nest_fs.mk      debug
	make -f nest_io.mk      debug
	make -f nest_itutil.mk  debug
	make -f nest_json.mk    debug
	make -f nest_math.mk    debug
	make -f nest_rand.mk    debug
	make -f nest_sequtil.mk debug
	make -f nest_sutil.mk   debug
	make -f nest_sys.mk     debug
	make -f nest_time.mk    debug
	make -f nest_utf8.mk    debug
