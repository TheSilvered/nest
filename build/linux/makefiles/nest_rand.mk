LIB_NAME = rand

CC = g++
CFLAGS = -I../../../include -Wall -shared -fPIC -rdynamic -std=c++2a
DBG_FLAGS = -D_DEBUG -g -O0
OUT_FILE = std$(LIB_NAME).so

x64_DIR = ../linux_release/x64
x86_DIR = ../linux_release/x86
DBG_DIR = ../linux_debug

CLINKS = -lm -ldl -lnest
CLINK_DIR_DBG := -L$(DBG_DIR)
CLINK_DIR_x86 := -L$(x86_DIR)
CLINK_DIR_x64 := -L$(x64_DIR)
CLINKS_DBG := $(CLINK_DIR_DBG) $(CLINKS)
CLINKS_x86 := $(CLINK_DIR_x86) $(CLINKS)
CLINKS_x64 := $(CLINK_DIR_x64) $(CLINKS)

SRCS := $(wildcard ../../../libs/nest_$(LIB_NAME)/*.cpp)
DBG_TARGET := $(DBG_DIR)/$(OUT_FILE)
x64_TARGET := $(x64_DIR)/$(OUT_FILE)
x86_TARGET := $(x86_DIR)/$(OUT_FILE)
NEST_LIB_DBG := $(DBG_DIR)/libnest.so
NEST_LIB_x64 := $(x64_DIR)/libnest.so
NEST_LIB_x86 := $(x86_DIR)/libnest.so

THIS_MK := nest_$(LIB_NAME).mk

.PHONY: debug x86 __no_libnest_x64 __no_libnest_x86 __no_libnest_dbg

main:
	make -f libnest.mk
	make -f $(THIS_MK) __no_libnest_x64
__no_libnest_x64: $(x64_TARGET);
$(x64_TARGET): $(SRCS) $(NEST_LIB_x64)
	mkdir -p $(x64_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x64) -O3 -o $(x64_TARGET)

x86:
	make -f libnest.mk x86
	make -f $(THIS_MK) __no_libnest_x86
__no_libnest_x86: $(x86_TARGET);
$(x86_TARGET): $(SRCS) $(NEST_LIB_x86)
	mkdir -p $(x86_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x86) -O3 -m32 -o $(x86_TARGET)

debug:
	make -f libnest.mk debug
	make -f $(THIS_MK) __no_libnest_dbg
__no_libnest_dbg: $(DBG_TARGET);
$(DBG_TARGET): $(SRCS) $(NEST_LIB_DBG)
	mkdir -p $(DBG_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS_DBG) $(DBG_FLAGS) -o $(DBG_TARGET)
