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

LIB_MAKEFILES = $(wildcard ./nest*.mk)

.PHONY: debug x86 clean all all-debug all-x86 __no_libnest_x64 __no_libnest_x86 __no_libnest_dbg

main:
	make -f libnest.mk
	make __no_libnest_x64
__no_libnest_x64: $(x64_TARGET);
$(x64_TARGET): $(SRCS)
	echo $(x64_TARGET)
	mkdir -p $(x64_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x64) -O3 -o $(x64_TARGET)

x86:
	make -f libnest.mk x86
	make __no_libnest_x86
__no_libnest_x86: $(x86_TARGET);
$(x86_TARGET): $(SRCS)
	mkdir -p $(x86_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x86) -O3 -m32 -o $(x86_TARGET)

debug:
	make -f libnest.mk debug
	make __no_libnest_dbg
__no_libnest_dbg: $(DBG_TARGET);
$(DBG_TARGET): $(SRCS) $(NEST_LIB_DBG)
	mkdir -p $(DBG_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS_DBG) $(DBG_FLAGS) -o $(DBG_TARGET)

clean:
	rm -fr $(EXE_DIR)
	rm -fr $(DBG_DIR)

all:
	make -f libnest.mk
	make                    __no_libnest_x64
	make -f nest_co.mk      __no_libnest_x64
	make -f nest_err.mk     __no_libnest_x64
	make -f nest_fs.mk      __no_libnest_x64
	make -f nest_gui.mk     __no_libnest_x64
	make -f nest_io.mk      __no_libnest_x64
	make -f nest_itutil.mk  __no_libnest_x64
	make -f nest_json.mk    __no_libnest_x64
	make -f nest_math.mk    __no_libnest_x64
	make -f nest_rand.mk    __no_libnest_x64
	make -f nest_sequtil.mk __no_libnest_x64
	make -f nest_sutil.mk   __no_libnest_x64
	make -f nest_sys.mk     __no_libnest_x64
	make -f nest_time.mk    __no_libnest_x64
	make -f nest_utf8.mk    __no_libnest_x64

all-x86:
	make -f libnest.mk      x86
	make                    __no_libnest_x86
	make -f nest_co.mk      __no_libnest_x86
	make -f nest_err.mk     __no_libnest_x86
	make -f nest_fs.mk      __no_libnest_x86
	make -f nest_gui.mk     __no_libnest_x86
	make -f nest_io.mk      __no_libnest_x86
	make -f nest_itutil.mk  __no_libnest_x86
	make -f nest_json.mk    __no_libnest_x86
	make -f nest_math.mk    __no_libnest_x86
	make -f nest_rand.mk    __no_libnest_x86
	make -f nest_sequtil.mk __no_libnest_x86
	make -f nest_sutil.mk   __no_libnest_x86
	make -f nest_sys.mk     __no_libnest_x86
	make -f nest_time.mk    __no_libnest_x86
	make -f nest_utf8.mk    __no_libnest_x86

all-debug:
	make -f libnest.mk      debug
	make                    __no_libnest_dbg
	make -f nest_co.mk      __no_libnest_dbg
	make -f nest_err.mk     __no_libnest_dbg
	make -f nest_fs.mk      __no_libnest_dbg
	make -f nest_gui.mk     __no_libnest_dbg
	make -f nest_io.mk      __no_libnest_dbg
	make -f nest_itutil.mk  __no_libnest_dbg
	make -f nest_json.mk    __no_libnest_dbg
	make -f nest_math.mk    __no_libnest_dbg
	make -f nest_rand.mk    __no_libnest_dbg
	make -f nest_sequtil.mk __no_libnest_dbg
	make -f nest_sutil.mk   __no_libnest_dbg
	make -f nest_sys.mk     __no_libnest_dbg
	make -f nest_time.mk    __no_libnest_dbg
	make -f nest_utf8.mk    __no_libnest_dbg
