MAKE_FILE = $(MAKE) --no-print-directory -f

CC = gcc
CFLAGS = $(LIB_FLAGS) -D_FILE_OFFSET_BITS=64 -I$(abspath ../../include)     \
		 -Wall -Wextra -Wnull-dereference -Wshadow -shared -fPIC -rdynamic     \
		 -std=c++2a $(CLARGS)
DBG_FLAGS = -D_DEBUG -g -O0
TARGET_NAME = std$(LIB_NAME).cnest

ifeq ($(CC),gcc)
    CFLAGS += -Wlogical-op -Wduplicated-cond -Wduplicated-branches
endif

REL_DIR = $(abspath linux_release)
DBG_DIR = $(abspath linux_debug)

CLINKS = -lm -ldl -lnest -lstdc++ $(LIB_LINKS)
CLINK_DIR_DBG := -L$(DBG_DIR)
CLINK_DIR_REL := -L$(REL_DIR)
CLINKS_DBG := $(CLINK_DIR_DBG) $(CLINKS)
CLINKS_REL := $(CLINK_DIR_REL) $(CLINKS)

SRCS := $(abspath $(wildcard ../../libs/nest_$(LIB_NAME)/*.cpp))
HEADERS := $(abspath $(wildcard ../../libs/nest_$(LIB_NAME)/*.h))
DBG_TARGET := $(DBG_DIR)/$(TARGET_NAME)
REL_TARGET := $(REL_DIR)/$(TARGET_NAME)
NEST_LIB_DBG := $(DBG_DIR)/libnest.so
NEST_LIB_REL := $(REL_DIR)/libnest.so

.PHONY: debug __no_libnest_rel __no_libnest_dbg help

main: $(REL_TARGET)

$(REL_TARGET): $(SRCS) $(HEADERS) $(NEST_LIB_REL)
	@if [ "$(LIB_NAME)" = "" ]; then echo LIB_NAME not defined.; exit 1; fi
	@mkdir -p $(REL_DIR)
	@echo "Compiling $(TARGET_NAME) for release..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_REL) -O3 -o $(REL_TARGET)

debug: $(DBG_TARGET)

$(DBG_TARGET): $(SRCS) $(HEADERS) $(NEST_LIB_DBG)
	@if [ "$(LIB_NAME)" = "" ]; then echo LIB_NAME not defined.; exit 1; fi
	@mkdir -p $(DBG_DIR)
	@echo "Compiling $(TARGET_NAME) in debug mode..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_DBG) $(DBG_FLAGS) -o $(DBG_TARGET)

help:
	@echo "stdlib.mk help:"
	@echo "  This file expects LIB_NAME to be defined except for 'help'. The library"
	@echo "  compiled is in the form 'std<LIB_NAME>.cnest', for example LIB_NAME=fs"
	@echo "  will compile 'stdfs.cnest'."
	@echo ""
	@echo "  make -f stdlib.mk LIB_NAME=libname"
	@echo "                            compile stdlibname.cnest for release"
	@echo "                            libnest.so is expected to be compiled"
	@echo "  make -f stdlib.mk debug LIB_NAME=libname"
	@echo "                            compile stdlibname.cnest with debug symbols"
	@echo "                            libnest.so is expected to be compiled""
	@echo ""
	@echo "  make -f stdlib.mk help    print this message"
