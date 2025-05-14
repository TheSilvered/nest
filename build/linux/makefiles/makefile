SILENT_MAKE = $(MAKE) --no-print-directory
MAKE_FILE = $(SILENT_MAKE) -f

ifndef CLARGS
CLARGS = $(REC_CLARGS)
endif
ORIG_CLAGS = $(CLARGS)

CC = gcc
CFLAGS = -D_FILE_OFFSET_BITS=64 -I$(abspath ../../../include) -Wall -Wextra \
		 -Wshadow -Wnull-dereference $(CLARGS)
DBG_FLAGS = -D_DEBUG -g -O0
TARGET_NAME = nest

ifeq ($(CC),gcc)
    CFLAGS += -Wlogical-op -Wduplicated-cond -Wduplicated-branches
endif

SRC_DIR = $(abspath ../../../src)
EXE_DIR = $(abspath ../linux_release)
x64_DIR:= $(EXE_DIR)/x64
x86_DIR:= $(EXE_DIR)/x86
DBG_DIR = $(abspath ../linux_debug)

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

RUN_FILE = _test.nest
RUN_DIR = ../../../tests

.PHONY: debug x86 all all-debug all-x86 help
.PHONY: __no_libnest_x64 __no_libnest_x86 __no_libnest_dbg
.PHONY: test test-x86 test-debug
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

run:
	@$(SILENT_MAKE) all
	@if [ -d ../linux_libs ]; then rm -r ../linux_libs/; fi
	@mkdir -p ../linux_libs
	@cd .. && ./_update_files.sh py
	@cp $(x64_DIR)/std*.cnest ../linux_libs
	@export NEST_LIBDIR=$(abspath ../linux_libs) \
	&& export LD_LIBRARY_PATH=$(x64_DIR):$LD_LIBRARY_PATH \
	&& cd $(RUN_DIR) \
	&& $(x64_DIR)/nest $(RUN_ARGS) $(RUN_FILE)

run-x86:
	@$(SILENT_MAKE) all-x86
	@if [ -d ../linux_libs ]; then rm -r ../linux_libs/; fi
	@mkdir -p ../linux_libs
	@cd .. && ./_update_files.sh py
	@cp $(x86_DIR)/std*.cnest ../linux_libs
	@export NEST_LIBDIR=$(abspath ../linux_libs) \
	&& export LD_LIBRARY_PATH=$(x86_DIR):$LD_LIBRARY_PATH \
	&& cd $(RUN_DIR) \
	&& $(x86_DIR)/nest $(RUN_ARGS) $(RUN_FILE)

run-debug:
	@$(SILENT_MAKE) all-debug
	@if [ -d ../linux_libs ]; then rm -r ../linux_libs/; fi
	@mkdir -p ../linux_libs
	@cd .. && ./_update_files.sh py
	@cp $(DBG_DIR)/std*.cnest ../linux_libs
	@export NEST_LIBDIR=$(abspath ../linux_libs) \
	&& export LD_LIBRARY_PATH=$(DBG_DIR):$LD_LIBRARY_PATH \
	&& cd $(RUN_DIR) \
	&& $(DEBUGGER) $(DBG_DIR)/nest $(RUN_ARGS) $(RUN_FILE)

test:
	@$(SILENT_MAKE) run RUN_FILE=run_tests.nest

test-x86:
	@$(SILENT_MAKE) run-x86 RUN_FILE=run_tests.nest

test-debug:
	@$(SILENT_MAKE) run-debug RUN_FILE=run_tests.nest

test-c:
	@$(MAKE_FILE) testnest.mk CLARGS=$(REC_CLARGS)
	@echo "Running test_nest..."
	@export LD_LIBRARY_PATH=. && echo $(x64_DIR) && cd $(x64_DIR) && ./test_nest

test-c-x86:
	@$(MAKE_FILE) testnest.mk x86 CLARGS=$(REC_CLARGS)
	@echo "Running test_nest..."
	@export LD_LIBRARY_PATH=. && cd $(x86_DIR) && ./test_nest

test-c-debug:
	@$(MAKE_FILE) testnest.mk debug CLARGS=$(REC_CLARGS)
	@echo "Running test_nest..."
	@export LD_LIBRARY_PATH=. \
		&& echo $(DBG_DIR) && \
		cd $(DBG_DIR) \
		&& DEBUGGER=$(DEBUGGER) ./test_nest

clear:
	@$(SILENT_MAKE) clean
clean:
	rm -frv $(EXE_DIR)
	rm -frv $(DBG_DIR)

all:
	@$(MAKE_FILE) libnest.mk CLARGS=$(REC_CLARGS)
	@$(SILENT_MAKE) __no_libnest_x64 CLARGS=$(ORIG_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=co      CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=codecs  CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=err     CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=fs      CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=io      CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=itutil  CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=json    CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=math    CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=rand    CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=sequtil CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=sutil   CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=sys     CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x64 LIB_NAME=time    CLARGS=$(REC_CLARGS)

all-x86:
	@$(MAKE_FILE) libnest.mk x86 CLARGS=$(REC_CLARGS)
	@$(SILENT_MAKE) __no_libnest_x86 CLARGS=$(ORIG_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=co      CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=codecs  CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=err     CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=fs      CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=io      CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=itutil  CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=json    CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=math    CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=rand    CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=sequtil CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=sutil   CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=sys     CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_x86 LIB_NAME=time    CLARGS=$(REC_CLARGS)

all-debug:
	@$(MAKE_FILE) libnest.mk debug CLARGS=$(REC_CLARGS)
	@$(SILENT_MAKE) __no_libnest_dbg CLARGS=$(ORIG_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=co      CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=codecs  CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=err     CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=fs      CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=io      CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=itutil  CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=json    CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=math    CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=rand    CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=sequtil CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=sutil   CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=sys     CLARGS=$(REC_CLARGS)
	@$(MAKE_FILE) stdlib.mk __no_libnest_dbg LIB_NAME=time    CLARGS=$(REC_CLARGS)

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
	@cd .. && bash ./uninstall.sh

help:
	@echo "makefile help:"
	@echo "  make                      compile nest and libnest.so for 64 bit platforms"
	@echo "  make x86                  compile nest and libnest.so for 32 bit platforms"
	@echo "  make debug                compile nest and libnest.so with debug symbols"
	@echo "  make all                  compile everything for 64 bit platforms"
	@echo "  make all-x86              compile everything for 32 bit platforms"
	@echo "  make all-debug            compile everything with debug symbols"
	@echo ""
	@echo "  make __no_libnest_x64     compile nest for 64 bit platforms"
	@echo "  make __no_libnest_x86     compile nest for 32 bit platforms"
	@echo "  make __no_libnest_dbg     compile nest with debug symbols"
	@echo ""
	@echo "  make test                 compile everything for 64 bit platforms and run"
	@echo "                            tests/run_tests.nest"
	@echo "  make test-x86             compile everything for 32 bit platforms and run"
	@echo "                            tests/run_tests.nest"
	@echo "  make test-debug           compile everything in debug mode and run"
	@echo "                            tests/run_tests.nest"
	@echo ""
	@echo "  make test-c               compile C tests for 64 bit platforms and run them"
	@echo "  make test-c-x86           compile C tests for 32 bit platforms and run them"
	@echo "  make test-c-debug         compile C tests with debug symbols and run them"
	@echo ""
	@echo "  make run                  compile everything for 64 bit platforms and run"
	@echo "                            RUN_FILE in RUN_DIR with RUN_ARGS"
	@echo "  make run-x86              compile everything for 32 bit platforms and run"
	@echo "                            RUN_FILE in RUN_DIR with RUN_ARGS"
	@echo "  make run-debug            compile everything in debug mode and run"
	@echo "                            RUN_FILE in RUN_DIR with RUN_ARGS"
	@echo ""
	@echo "  make install              run 'make all' and 'linux_install_x64.sh'"
	@echo "  make install-x86          run 'make all-x86' and 'linux_install_x86.sh'"
	@echo "  make install-debug        run 'make all-debug' and '_install_debug.sh'"
	@echo "  make uninstall            run 'uninstall.sh'"
	@echo ""
	@echo "  make clean, make clear    remove the output folders"
	@echo ""
	@echo "  make help                 print this message"
	@echo ""
	@echo "Variables:"
	@echo "  CC                        by default set to gcc, can be set to clang"
	@echo "  CLARGS                    additional arguments passed when compiling nest,"
	@echo "                            it overwrites REC_CLARGS"
	@echo "  REC_CLARGS                additional arguments passed when compiling"
	@echo "                            libnest.so and the libraries, they are also passed"
	@echo "                            when compiling nest if CLARGS is not set"
	@echo "  DEBUGGER                  program used to run nest when executing"
	@echo "                            'make run-debug' and 'make test-c-debug'"
	@echo "  RUN_FILE                  the file to run with 'make run*', by default it is"
	@echo "                            <nest dir>/tests/_test.nest"
	@echo "  RUN_DIR                   the current working directory for 'make run*', by"
	@echo "                            default it is <nest dir>/tests/"
	@echo "  RUN_ARGS                  the arguments passed to Nest with 'make run*'"
