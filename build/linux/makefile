SILENT_MAKE = $(MAKE) --no-print-directory
MAKE_FILE = $(SILENT_MAKE) -f

CC = gcc
CFLAGS = -D_FILE_OFFSET_BITS=64 -I$(abspath ../../include) -Wall -Wextra -Wshadow $(CLARGS)
DBG_FLAGS = -D_DEBUG -g -O0

ifeq ($(CC),gcc)
    CFLAGS += -Wlogical-op -Wduplicated-cond -Wduplicated-branches
endif

ifndef INSTALL_LIB_PREFIX
INSTALL_LIB_PREFIX = /usr/lib
endif

ifndef INSTALL_EXE_PREFIX
INSTALL_EXE_PREFIX = /usr/bin
endif

SRC_DIR := $(abspath ../../src)
REL_DIR := $(abspath linux_release)
DBG_DIR := $(abspath linux_debug)
TEST_SRC_DIR := $(abspath ../../tests/test_nest)

STDLIB_DIR := $(abspath linux_libs)

CLINKS = -lm -ldl
CLINKS_DBG := -L$(DBG_DIR) $(CLINKS) -lnest
CLINKS_REL := -L$(REL_DIR) $(CLINKS) -lnest

EXE_SRCS := $(SRC_DIR)/nest.c
LIB_SRCS := $(filter-out $(SRC_DIR)/nest.c, $(wildcard $(SRC_DIR)/*.c))
LIB_HEADERS := $(wildcard $(INCLUDE_DIR)/*.h)
TEST_SRCS := $(wildcard $(TEST_SRC_DIR)/*.c)

EXE_TARGET_NAME = nest
LIB_TARGET_NAME = libnest.so
TEST_TARGET_NAME = test_nest

REL_EXE_TARGET := $(REL_DIR)/$(EXE_TARGET_NAME)
DBG_EXE_TARGET := $(DBG_DIR)/$(EXE_TARGET_NAME)
REL_LIB_TARGET := $(REL_DIR)/$(LIB_TARGET_NAME)
DBG_LIB_TARGET := $(DBG_DIR)/$(LIB_TARGET_NAME)
REL_TEST_TARGET := $(REL_DIR)/$(TEST_TARGET_NAME)
DBG_TEST_TARGET := $(DBG_DIR)/$(TEST_TARGET_NAME)

ifndef RUN_FILE
RUN_FILE = _test.nest
endif

ifndef RUN_DIR
RUN_DIR = ../../tests
endif

.PHONY: debug all all-debug help
.PHONY: test test-release
.PHONY: clean clear
.PHONY: run run-debug
.PHONY: install uninstall

main: $(REL_EXE_TARGET)

$(REL_EXE_TARGET): $(EXE_SRCS) $(REL_LIB_TARGET)
	@echo "Compiling $(EXE_TARGET_NAME) for release..."
	@$(CC) $(CFLAGS) -O3 $(EXE_SRCS) $(CLINKS_REL) -o $(REL_EXE_TARGET)

$(REL_LIB_TARGET): $(LIB_SRCS) $(LIB_HEADERS)
	@mkdir -p $(REL_DIR)
	@echo "Compiling $(LIB_TARGET_NAME) for release..."
	@$(CC) $(CFLAGS) -shared -fPIC -rdynamic -O3 $(LIB_SRCS) $(CLINKS) -o $(REL_LIB_TARGET)

debug: $(DBG_EXE_TARGET)

$(DBG_EXE_TARGET): $(EXE_SRCS) $(DBG_LIB_TARGET)
	@mkdir -p $(DBG_DIR)
	@echo "Compiling $(EXE_TARGET_NAME) in debug mode..."
	@$(CC) $(CFLAGS) $(DBG_FLAGS) $(EXE_SRCS) $(CLINKS_DBG) -o $(DBG_EXE_TARGET)

$(DBG_LIB_TARGET): $(LIB_SRCS) $(LIB_HEADERS)
	@mkdir -p $(DBG_DIR)
	@echo "Compiling $(LIB_TARGET_NAME) in debug mode..."
	@$(CC) $(CFLAGS) $(DBG_FLAGS) -shared -fPIC -rdynamic $(LIB_SRCS) $(CLINKS) -o $(DBG_LIB_TARGET)

run:
	@$(SILENT_MAKE) all
	@mkdir -p $(STDLIB_DIR)
	@sh ./update_files.sh py
	@cp $(REL_DIR)/std*.cnest $(STDLIB_DIR)
	@export NEST_LIBDIR=$(abspath $(STDLIB_DIR)) \
	&& export LD_LIBRARY_PATH=$(REL_DIR):$LD_LIBRARY_PATH \
	&& cd $(RUN_DIR) \
	&& $(REL_DIR)/nest $(RUN_ARGS) $(RUN_FILE)

run-debug:
	@$(SILENT_MAKE) all-debug
	@mkdir -p $(STDLIB_DIR)
	@sh ./update_files.sh py
	@cp $(DBG_DIR)/std*.cnest $(STDLIB_DIR)
	@export NEST_LIBDIR=$(abspath $(STDLIB_DIR)) \
	&& export LD_LIBRARY_PATH=$(DBG_DIR):$LD_LIBRARY_PATH \
	&& cd $(RUN_DIR) \
	&& $(DEBUGGER) $(DBG_DIR)/nest $(RUN_ARGS) $(RUN_FILE)

test-release: $(REL_TEST_TARGET)
	@$(SILENT_MAKE) run RUN_FILE=run_tests.nest
	@export NEST_LIBDIR=$(abspath $(STDLIB_DIR)) \
		&& export LD_LIBRARY_PATH=$(REL_DIR):$LD_LIBRARY_PATH \
		&& $(REL_TEST_TARGET)

$(REL_TEST_TARGET): $(TEST_SRCS) $(REL_LIB_TARGET)
	@echo "Compiling $(TEST_TARGET_NAME) for release..."
	@$(CC) $(CFLAGS) $(TEST_SRCS) $(CLINKS_REL) -O3 -o $(REL_TEST_TARGET)

test: $(DBG_TEST_TARGET)
	@$(SILENT_MAKE) run-debug RUN_FILE=run_tests.nest
	@export NEST_LIBDIR=$(abspath $(STDLIB_DIR)) \
		&& export LD_LIBRARY_PATH=$(DBG_DIR):$LD_LIBRARY_PATH \
		&& $(DBG_TEST_TARGET)

$(DBG_TEST_TARGET): $(TEST_SRCS) $(DBG_LIB_TARGET)
	@echo "Compiling $(TEST_TARGET_NAME) in debug mode..."
	@$(CC) $(CFLAGS) $(DBG_FLAGS) $(TEST_SRCS) $(CLINKS_DBG) -o $(DBG_TEST_TARGET)

clear:
	@$(SILENT_MAKE) clean
clean:
	rm -frv $(REL_DIR)
	rm -frv $(DBG_DIR)

all:
	@$(SILENT_MAKE)
	@$(MAKE_FILE) stdlib.mk LIB_NAME=co
	@$(MAKE_FILE) stdlib.mk LIB_NAME=codecs
	@$(MAKE_FILE) stdlib.mk LIB_NAME=err
	@$(MAKE_FILE) stdlib.mk LIB_NAME=fs
	@$(MAKE_FILE) stdlib.mk LIB_NAME=io
	@$(MAKE_FILE) stdlib.mk LIB_NAME=itutil
	@$(MAKE_FILE) stdlib.mk LIB_NAME=json
	@$(MAKE_FILE) stdlib.mk LIB_NAME=math
	@$(MAKE_FILE) stdlib.mk LIB_NAME=rand
	@$(MAKE_FILE) stdlib.mk LIB_NAME=sequtil
	@$(MAKE_FILE) stdlib.mk LIB_NAME=sutil
	@$(MAKE_FILE) stdlib.mk LIB_NAME=sys
	@$(MAKE_FILE) stdlib.mk LIB_NAME=time
	@sh ./update_files.sh py

all-debug:
	@$(SILENT_MAKE) debug
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=co
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=codecs
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=err
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=fs
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=io
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=itutil
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=json
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=math
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=rand
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=sequtil
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=sutil
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=sys
	@$(MAKE_FILE) stdlib.mk debug LIB_NAME=time
	@sh ./update_files.sh py

install:
	@$(SILENT_MAKE) all
	@cd .. && ./update_files.sh
	@sudo mkdir -p $(INSTALL_LIB_PREFIX)/nest/
	@sudo mkdir -p $(INSTALL_EXE_PREFIX)/
	@sudo cp -r $(STDLIB_DIR)/* $(INSTALL_LIB_PREFIX)/nest
	@sudo cp $(REL_DIR)/*.cnest $(INSTALL_LIB_PREFIX)/nest
	@sudo cp $(REL_DIR)/nest $(INSTALL_EXE_PREFIX)/
	@sudo cp $(REL_DIR)/libnest.so $(INSTALL_LIB_PREFIX)/
	@sudo ldconfig

uninstall:
	@bash ./uninstall.sh

help:
	@echo "makefile help:"
	@echo "  make                      compile nest for relase"
	@echo "  make debug                compile nest with debug symbols"
	@echo "  make all                  compile everything for relase"
	@echo "  make all-debug            compile everything with debug symbols"
	@echo ""
	@echo "  make test                 compile and run the tests in debug mode"
	@echo "  make test-release         compile and run the tests in release mode"
	@echo ""
	@echo "  make run                  compile everything for release and run a file"
	@echo "  make run-debug            compile everything in debug mode and run a file"
	@echo ""
	@echo "  make install              compile for release and install nest"
	@echo "  make uninstall            uninstall nest"
	@echo ""
	@echo "  make clean, make clear    remove the output folders"
	@echo ""
	@echo "  make help                 print this message"
	@echo ""
	@echo "Variables:"
	@echo "  CC                        by default set to gcc, can be set to clang"
	@echo "  CLARGS                    additional arguments passed when compiling nest"
	@echo "  DEBUGGER                  program used to run nest with 'run-debug'"
	@echo "  RUN_FILE                  the file to run with 'run' and 'run-debug', by"
	@echo "                            default it is <nest dir>/tests/_test.nest"
	@echo "  RUN_DIR                   the current working directory for 'run' and"
	@echo "                            'run-debug', by default it is <nest dir>/tests/"
	@echo "  RUN_ARGS                  the arguments passed to nest with 'run' and"
	@echo "                            'run-debug'"
