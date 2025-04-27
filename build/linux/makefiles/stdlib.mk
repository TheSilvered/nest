MAKE_FILE = $(MAKE) --no-print-directory -f

CC = gcc
CFLAGS = $(LIB_FLAGS) -D_FILE_OFFSET_BITS=64 -I$(abspath ../../../include)     \
		 -Wall -Wextra -Wnull-dereference -Wshadow -shared -fPIC -rdynamic     \
		 -std=c++2a
DBG_FLAGS = -D_DEBUG -g -O0
TARGET_NAME = std$(LIB_NAME).cnest

ifeq ($(CC),gcc)
    CFLAGS += -Wlogical-op -Wduplicated-cond -Wduplicated-branches
endif

x64_DIR = $(abspath ../linux_release/x64)
x86_DIR = $(abspath ../linux_release/x86)
DBG_DIR = $(abspath ../linux_debug)

CLINKS = -lm -ldl -lnest -lstdc++ $(LIB_LINKS)
CLINK_DIR_DBG := -L$(DBG_DIR)
CLINK_DIR_x86 := -L$(x86_DIR)
CLINK_DIR_x64 := -L$(x64_DIR)
CLINKS_DBG := $(CLINK_DIR_DBG) $(CLINKS)
CLINKS_x86 := $(CLINK_DIR_x86) $(CLINKS)
CLINKS_x64 := $(CLINK_DIR_x64) $(CLINKS)

SRCS := $(abspath $(wildcard ../../../libs/nest_$(LIB_NAME)/*.cpp))
HEADERS := $(abspath $(wildcard ../../../libs/nest_$(LIB_NAME)/*.h))
DBG_TARGET := $(DBG_DIR)/$(TARGET_NAME)
x64_TARGET := $(x64_DIR)/$(TARGET_NAME)
x86_TARGET := $(x86_DIR)/$(TARGET_NAME)
NEST_LIB_DBG := $(DBG_DIR)/libnest.so
NEST_LIB_x64 := $(x64_DIR)/libnest.so
NEST_LIB_x86 := $(x86_DIR)/libnest.so

.PHONY: debug x86 __no_libnest_x64 __no_libnest_x86 __no_libnest_dbg help

main:
	@if [ "$(LIB_NAME)" = "" ]; then echo LIB_NAME not defined.; exit 1; fi
	@$(MAKE_FILE) libnest.mk
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 -LIB_NAME=$(LIB_NAME) \
		-LIB_FLAGS=$(LIB_FLAGS) -LIB_LINKS=$(LIB_LINKS)
__no_libnest_x64: $(x64_TARGET);
$(x64_TARGET): $(SRCS) $(HEADERS) $(NEST_LIB_x64)
	@if [ "$(LIB_NAME)" = "" ]; then echo LIB_NAME not defined.; exit 1; fi
	@mkdir -p $(x64_DIR)
	@echo "Compiling $(TARGET_NAME) for x64..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x64) -O3 -o $(x64_TARGET)

x86:
	@if [ "$(LIB_NAME)" = "" ]; then echo LIB_NAME not defined.; exit 1; fi
	@$(MAKE_FILE) libnest.mk x86
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 -LIB_NAME=$(LIB_NAME) \
		-LIB_FLAGS=$(LIB_FLAGS) -LIB_LINKS=$(LIB_LINKS)
__no_libnest_x86: $(x86_TARGET);
$(x86_TARGET): $(SRCS) $(HEADERS) $(NEST_LIB_x86)
	@if [ "$(LIB_NAME)" = "" ]; then echo LIB_NAME not defined.; exit 1; fi
	@mkdir -p $(x86_DIR)
	@echo "Compiling $(TARGET_NAME) for x86..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x86) -O3 -m32 -o $(x86_TARGET)

debug:
	@if [ "$(LIB_NAME)" = "" ]; then echo LIB_NAME not defined.; exit 1; fi
	@$(MAKE_FILE) libnest.mk debug
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg -LIB_NAME=$(LIB_NAME) \
		-LIB_FLAGS=$(LIB_FLAGS) -LIB_LINKS=$(LIB_LINKS)
__no_libnest_dbg: $(DBG_TARGET);
$(DBG_TARGET): $(SRCS) $(HEADERS) $(NEST_LIB_DBG)
	@if [ "$(LIB_NAME)" = "" ]; then echo LIB_NAME not defined.; exit 1; fi
	@mkdir -p $(DBG_DIR)
	@echo "Compiling $(TARGET_NAME) in debug mode..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_DBG) $(DBG_FLAGS) -o $(DBG_TARGET)

help:
	@echo "stdlib.mk help:"
	@echo "  This file expects LIB_NAME to be defined except for 'help'. The library"
	@echo "  compiled is in the form 'std<LIB_NAME>.cnest', for example LIB_NAME=fs"
	@echo "  will compile stdfs.cnest."
	@echo ""
	@echo "  make -f stdlib.mk LIB_NAME=libname"
	@echo "                             compile libnest.so and stdlibname.cnest for 64 bit"
	@echo "                             platforms"
	@echo "  make -f stdlib.mk x86 LIB_NAME=libname"
	@echo "                             compile libnest.so and stdlibname.cnest for 32 bit"
	@echo "                             platforms"
	@echo "  make -f stdlib.mk debug LIB_NAME=libname"
	@echo "                             compile libnest.so and stdlibname.cnest for 64 with"
	@echo "                             debug symbols"
	@echo ""
	@echo "  make -f stdlib.mk __no_libnest_x64 LIB_NAME=libname"
	@echo "                             compile stdlibname.cnest for 64 bit platforms"
	@echo "  make -f stdlib.mk __no_libnest_x86 LIB_NAME=libname"
	@echo "                             compile stdlibname.cnest for 32 bit platforms"
	@echo "  make -f stdlib.mk __no_libnest_dbg LIB_NAME=libname"
	@echo "                             compile stdlibname.cnest with debug symbols"
	@echo ""
	@echo "  make -f stdlib.mk help     print this message"
