ifeq ($(LIB_NAME),gui)
  LIB_FLAGS=$(shell pkg-config --cflags sdl2 SDL2_ttf)
  LIB_LINKS=$(shell pkg-config --libs sdl2 SDL2_ttf)
endif


MAKE_FILE = $(MAKE) --no-print-directory -f

CC = g++
CFLAGS = $(LIB_FLAGS) -I../../../include -Wall -Wextra -Wlogical-op -Wnull-dereference -Wduplicated-cond -Wduplicated-branches -Wshadow -shared -fPIC -rdynamic -std=c++2a
DBG_FLAGS = -D_DEBUG -g -O0
TARGET_NAME = std$(LIB_NAME).cnest

x64_DIR = ../linux_release/x64
x86_DIR = ../linux_release/x86
DBG_DIR = ../linux_debug

CLINKS = -lm -ldl -lnest $(LIB_LINKS)
CLINK_DIR_DBG := -L$(DBG_DIR)
CLINK_DIR_x86 := -L$(x86_DIR)
CLINK_DIR_x64 := -L$(x64_DIR)
CLINKS_DBG := $(CLINK_DIR_DBG) $(CLINKS)
CLINKS_x86 := $(CLINK_DIR_x86) $(CLINKS)
CLINKS_x64 := $(CLINK_DIR_x64) $(CLINKS)

SRCS := $(wildcard ../../../libs/nest_$(LIB_NAME)/*.cpp)
HEADERS := $(wildcard ../../../libs/nest_$(LIB_NAME)/*.h)
DBG_TARGET := $(DBG_DIR)/$(TARGET_NAME)
x64_TARGET := $(x64_DIR)/$(TARGET_NAME)
x86_TARGET := $(x86_DIR)/$(TARGET_NAME)
NEST_LIB_DBG := $(DBG_DIR)/libnest.so
NEST_LIB_x64 := $(x64_DIR)/libnest.so
NEST_LIB_x86 := $(x86_DIR)/libnest.so

.PHONY: debug x86 __no_libnest_x64 __no_libnest_x86 __no_libnest_dbg help

main:
	@$(MAKE_FILE) libnest.mk
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 -LIB_NAME=$(LIB_NAME) \
		-LIB_FLAGS=$(LIB_FLAGS) -LIB_LINKS=$(LIB_LINKS)
__no_libnest_x64: $(x64_TARGET);
$(x64_TARGET): $(SRCS) $(HEADERS) $(NEST_LIB_x64)
	@mkdir -p $(x64_DIR)
	@echo "Compiling $(TARGET_NAME) for x64..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x64) -O3 -o $(x64_TARGET)

x86:
	@$(MAKE_FILE) libnest.mk x86
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 -LIB_NAME=$(LIB_NAME) \
		-LIB_FLAGS=$(LIB_FLAGS) -LIB_LINKS=$(LIB_LINKS)
__no_libnest_x86: $(x86_TARGET);
$(x86_TARGET): $(SRCS) $(HEADERS) $(NEST_LIB_x86)
	@mkdir -p $(x86_DIR)
	@echo "Compiling $(TARGET_NAME) for x86..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x86) -O3 -m32 -o $(x86_TARGET)

debug:
	@$(MAKE_FILE) libnest.mk debug
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg -LIB_NAME=$(LIB_NAME) \
		-LIB_FLAGS=$(LIB_FLAGS) -LIB_LINKS=$(LIB_LINKS)
__no_libnest_dbg: $(DBG_TARGET);
$(DBG_TARGET): $(SRCS) $(HEADERS) $(NEST_LIB_DBG)
	@mkdir -p $(DBG_DIR)
	@echo "Compiling $(TARGET_NAME) in debug mode..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_DBG) $(DBG_FLAGS) -o $(DBG_TARGET)

help:
	@echo "stdlib.mk help:"
	@echo "  This file always expects LIB_NAME to be defined except for help. It is the"
	@echo "  name of the library that will be compiled without 'std' at the front. For"
	@echo "  example LIB_NAME=fs will compile stdfs.cnest, LIB_NAME=io will compile"
	@echo "  stdio.cnest and so on."
	@echo ""
	@echo "  make -f stdlib.mk                   compile libnest.so for 64 bit platforms"
	@echo "  make -f stdlib.mk x86               compile libnest.so for 32 bit platforms"
	@echo "  make -f stdlib.mk debug             compile libnest.so with debug symbols"
	@echo ""
	@echo "  make -f stdlib.mk __no_libnest_x64  compile libnest.so with debug symbols"
	@echo "  make -f stdlib.mk __no_libnest_x86  compile libnest.so with debug symbols"
	@echo "  make -f stdlib.mk __no_libnest_dbg  compile libnest.so with debug symbols"
	@echo ""
	@echo "  make -f stdlib.mk help              prints this message"
