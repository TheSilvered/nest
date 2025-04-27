CC = gcc
CFLAGS = -D_FILE_OFFSET_BITS=64 -I$(abspath ../../../include) -Wall -Wextra    \
		 -Wshadow -shared -fPIC -rdynamic
DBG_FLAGS = -D_DEBUG -g -O0
TARGET_NAME = libnest.so

ifneq ($(CC),clang)
    CFLAGS += -Wlogical-op -Wduplicated-cond -Wduplicated-branches
endif

INCLUDE_DIR = $(abspath ../../../include)
SRC_DIR = $(abspath ../../../src)
x64_DIR = $(abspath ../linux_release/x64)
x86_DIR = $(abspath ../linux_release/x86)
DBG_DIR = $(abspath ../linux_debug)

CLINKS = -lm -ldl

SRCS := $(filter-out $(SRC_DIR)/nest.c, $(wildcard $(SRC_DIR)/*.c))
HEADERS := $(wildcard $(INCLUDE_DIR)/*.h)
DBG_TARGET := $(DBG_DIR)/$(TARGET_NAME)
x64_TARGET := $(x64_DIR)/$(TARGET_NAME)
x86_TARGET := $(x86_DIR)/$(TARGET_NAME)

.PHONY: debug x86 help

$(x64_TARGET): $(SRCS) $(HEADERS)
	@mkdir -p $(x64_DIR)
	@echo "Compiling $(TARGET_NAME) for x64..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $(x64_TARGET)

x86: $(x86_TARGET);
$(x86_TARGET): $(SRCS) $(HEADERS)
	@mkdir -p $(x86_DIR)
	@echo "Compiling $(TARGET_NAME) for x86..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -m32 -o $(x86_TARGET)

debug: $(DBG_TARGET);
$(DBG_TARGET): $(SRCS) $(HEADERS)
	@mkdir -p $(DBG_DIR)
	@echo "Compiling $(TARGET_NAME) in debug mode..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS) $(DBG_FLAGS) -o $(DBG_TARGET)

help:
	@echo "libnest.mk help:"
	@echo "  make -f libnest.mk         compile libnest.so for 64 bit platforms"
	@echo "  make -f libnest.mk x86     compile libnest.so for 32 bit platforms"
	@echo "  make -f libnest.mk debug   compile libnest.so with debug symbols"
	@echo ""
	@echo "  make -f libnest.mk help    print this message"
