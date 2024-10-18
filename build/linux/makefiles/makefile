SILENT_MAKE = $(MAKE) --no-print-directory
MAKE_FILE = $(SILENT_MAKE) -f

CC = gcc
CFLAGS = -D_FILE_OFFSET_BITS=64 -I../../../include -Wall -Wextra -Wshadow     \
         -Wnull-dereference
DBG_FLAGS = -D_DEBUG -g -O0
TARGET_NAME = nest

ifneq ($(CC),clang)
    CFLAGS += -Wlogical-op -Wduplicated-cond -Wduplicated-branches
endif

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
DBG_TARGET := $(DBG_DIR)/$(TARGET_NAME)
x64_TARGET := $(x64_DIR)/$(TARGET_NAME)
x86_TARGET := $(x86_DIR)/$(TARGET_NAME)
NEST_LIB_DBG := $(DBG_DIR)/libnest.so
NEST_LIB_x64 := $(x64_DIR)/libnest.so
NEST_LIB_x86 := $(x86_DIR)/libnest.so

.PHONY: debug x86 all all-debug all-x86 help
.PHONY: __no_libnest_x64 __no_libnest_x86 __no_libnest_dbg
.PHONY: test-c test-c-x86 test-c-debug
.PHONY: clean clear
.PHONY: run
.PHONY: install install-x86 install-debug uninstall

main:
	@$(MAKE_FILE) libnest.mk
	@$(SILENT_MAKE) __no_libnest_x64
__no_libnest_x64: $(x64_TARGET);
$(x64_TARGET): $(SRCS) $(NEST_LIB_x64)
	@mkdir -p $(x64_DIR)
	@echo "Compiling $(TARGET_NAME) for x64..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x64) -O3 -o $(x64_TARGET)

x86:
	@$(MAKE_FILE) libnest.mk x86
	@$(SILENT_MAKE) __no_libnest_x86
__no_libnest_x86: $(x86_TARGET);
$(x86_TARGET): $(SRCS) $(NEST_LIB_x86)
	@mkdir -p $(x86_DIR)
	@echo "Compiling $(TARGET_NAME) for x86..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x86) -O3 -m32 -o $(x86_TARGET)

debug:
	@$(MAKE_FILE) libnest.mk debug
	@$(SILENT_MAKE) __no_libnest_dbg
__no_libnest_dbg: $(DBG_TARGET);
$(DBG_TARGET): $(SRCS) $(NEST_LIB_DBG)
	@mkdir -p $(DBG_DIR)
	@echo "Compiling $(TARGET_NAME) in debug mode..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_DBG) $(DBG_FLAGS) -o $(DBG_TARGET)

test-c:
	@$(MAKE_FILE) testnest.mk
	@echo "Running test_nest..."
	@export LD_LIBRARY_PATH=. && echo $(x64_DIR) && cd $(x64_DIR) && ./test_nest

test-c-x86:
	@$(MAKE_FILE) testnest.mk x86
	@echo "Running test_nest..."
	@export LD_LIBRARY_PATH=. && cd $(x86_DIR) && ./test_nest

test-c-debug:
	@$(MAKE_FILE) testnest.mk debug
	@echo "Running test_nest..."
	@export LD_LIBRARY_PATH=. && echo $(DBG_DIR) && cd $(DBG_DIR) && ./test_nest

clear:
	@$(SILENT_MAKE) clean
clean:
	rm -frv $(EXE_DIR)
	rm -frv $(DBG_DIR)

all:
	@$(MAKE_FILE) libnest.mk
	@$(SILENT_MAKE) __no_libnest_x64
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=co
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=codecs
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=err
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=fs
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=gui
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=io
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=itutil
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=json
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=math
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=rand
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=sequtil
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=sutil
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=sys
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=time

all-x86:
	@$(MAKE_FILE) libnest.mk x86
	@$(SILENT_MAKE) __no_libnest_x86
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=co
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=codecs
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=err
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=fs
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=gui
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=io
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=itutil
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=json
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=math
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=rand
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=sequtil
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=sutil
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=sys
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=time

all-debug:
	@$(MAKE_FILE) libnest.mk debug
	@$(SILENT_MAKE) __no_libnest_dbg
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=co
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=codecs
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=err
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=fs
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=gui
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=io
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=itutil
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=json
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=math
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=rand
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=sequtil
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=sutil
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=sys
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=time

run:
	@$(SILENT_MAKE) all-debug
	@cd .. && ./_update_files.sh py && ./_install_debug.sh
	@cd ../../../tests && nest _test.nest

install:
	@$(SILENT_MAKE) all
	@cd .. && ./_update_files.sh py && ./linux_install_x64.sh

install-x86:
	@$(SILENT_MAKE) all-x86
	@cd .. && ./_update_files.sh py && ./linux_install_x86.sh

install-debug:
	@$(SILENT_MAKE) all-debug
	@cd .. && ./_update_files.sh py && ./_install_debug.sh

uninstall:
	@cd .. && ./uninstall.sh

help:
	@echo "makefile help:"
	@echo "  make                       compile nest and libnest.so for 64 bit platforms"
	@echo "  make x86                   compile nest and libnest.so for 32 bit platforms"
	@echo "  make debug                 compile nest and libnest.so with debug symbols"
	@echo "  make all                   compile everything for 64 bit platforms"
	@echo "  make all-x86               compile everything for 32 bit platforms"
	@echo "  make all-debug             compile everything with debug symbols"
	@echo ""
	@echo "  make __no_libnest_x64      compile nest for 64 bit platforms"
	@echo "  make __no_libnest_x86      compile nest for 32 bit platforms"
	@echo "  make __no_libnest_dbg      compile nest with debug symbols"
	@echo ""
	@echo "  make test-c                compile C tests for 64 bit platforms and run them"
	@echo "  make test-c-x86            compile C tests for 32 bit platforms and run them"
	@echo "  make test-c-debug          compile C tests with debug symbols and run them"
	@echo ""
	@echo "  make run                   run 'make all-debug', '_update_files.sh',"
	@echo "                             '_install_debug.sh' and then run '_test.nest'"
	@echo "                             from 'nest/tests/'; the file needs to be created"
	@echo ""
	@echo "  make install               run 'make all' and 'linux_install_x64.sh'"
	@echo "  make install-x86           run 'make all-x86' and 'linux_install_x86.sh'"
	@echo "  make install-debug         run 'make all-debug' and '_install_debug.sh'"
	@echo "  make uninstall             run 'uninstall.sh'"
	@echo ""
	@echo "  make clean, make clear     remove the output folders"
	@echo ""
	@echo "  make help                  print this message"
	@echo ""
	@$(MAKE_FILE) libnest.mk help
	@echo ""
	@$(MAKE_FILE) stdlib.mk help
	@echo ""
	@$(MAKE_FILE) testnest.mk help
	@echo ""
	@echo ""
	@echo "By default this makefile uses gcc. To compile with clang add 'CC=clang' to the"
	@echo "end of the command (e.g. 'make all CC=clang')."
