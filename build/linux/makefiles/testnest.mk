SILENT_MAKE = $(MAKE) --no-print-directory
MAKE_FILE = $(SILENT_MAKE) -f

CC = gcc
CFLAGS = -I$(abspath ../../../include) -Wall -Wextra -Wnull-dereference -Wshadow
DBG_FLAGS = -D_DEBUG -g -O0
TARGET_NAME = test_nest

ifeq ($(CC),gcc)
    CFLAGS += -Wlogical-op -Wduplicated-cond -Wduplicated-branches
endif

SRC_DIR = $(abspath ../../../tests/test_nest)
EXE_DIR = $(abspath ../linux_release)
x64_DIR:= $(EXE_DIR)/x64
x86_DIR:= $(EXE_DIR)/x86
DBG_DIR = $(abspath ../linux_debug)

CLINKS = -lnest
CLINK_DIR_DBG := -L$(DBG_DIR)
CLINK_DIR_x86 := -L$(x86_DIR)
CLINK_DIR_x64 := -L$(x64_DIR)
CLINKS_DBG := $(CLINK_DIR_DBG) $(CLINKS)
CLINKS_x86 := $(CLINK_DIR_x86) $(CLINKS)
CLINKS_x64 := $(CLINK_DIR_x64) $(CLINKS)

SRCS = $(wildcard $(SRC_DIR)/*.c)
DBG_TARGET := $(DBG_DIR)/$(TARGET_NAME)
x64_TARGET := $(x64_DIR)/$(TARGET_NAME)
x86_TARGET := $(x86_DIR)/$(TARGET_NAME)
NEST_LIB_DBG := $(DBG_DIR)/libnest.so
NEST_LIB_x64 := $(x64_DIR)/libnest.so
NEST_LIB_x86 := $(x86_DIR)/libnest.so

.PHONY: debug x86 help
.PHONY: __no_libnest_x64 __no_libnest_x86 __no_libnest_dbg

main:
	@$(MAKE_FILE) libnest.mk
	@$(MAKE_FILE) testnest.mk __no_libnest_x64
__no_libnest_x64: $(x64_TARGET);
$(x64_TARGET): $(SRCS) $(NEST_LIB_x64)
	@mkdir -p $(x64_DIR)
	@echo "Compiling $(TARGET_NAME) for x64..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x64) -O3 -o $(x64_TARGET)

x86:
	@$(MAKE_FILE) libnest.mk x86
	@$(MAKE_FILE) testnest.mk __no_libnest_x86
__no_libnest_x86: $(x86_TARGET);
$(x86_TARGET): $(SRCS) $(NEST_LIB_x86)
	@mkdir -p $(x86_DIR)
	@echo "Compiling $(TARGET_NAME) for x86..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x86) -O3 -m32 -o $(x86_TARGET)

debug:
	@$(MAKE_FILE) libnest.mk debug
	@$(MAKE_FILE) testnest.mk __no_libnest_dbg
__no_libnest_dbg: $(DBG_TARGET);
$(DBG_TARGET): $(SRCS) $(NEST_LIB_DBG)
	@mkdir -p $(DBG_DIR)
	@echo "Compiling $(TARGET_NAME) in debug mode..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_DBG) $(DBG_FLAGS) -o $(DBG_TARGET)

help:
	@echo "testnest.mk help:"
	@echo "  make -f testnest.mk        compile C tests and libnest.so for 64 bit platforms"
	@echo "  make -f testnest.mk x86    compile C tests and libnest.so for 32 bit platforms"
	@echo "  make -f testnest.mk debug  compile C tests and libnest.so with debug symbols"
	@echo ""
	@echo "  make -f testnest.mk __no_libnest_x64"
	@echo "                             compile C tests for 64 bit platforms"
	@echo "  make -f testnest.mk __no_libnest_x86"
	@echo "                             compile C tests for 32 bit platforms"
	@echo "  make -f testnest.mk __no_libnest_dbg"
	@echo "                             compile C tests with debug symbols"
	@echo ""
	@echo "  make -f testnest.mk help    print this message"
