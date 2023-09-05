LIB_NAME := vec_lib

CC=g++
CFLAGS = $(LIB_FLAGS) -D_FILE_OFFSET_BITS=64 -Inest_source -Wall -Wextra    \
         -Wlogical-op -Wnull-dereference -Wduplicated-cond -Wshadow -shared \
         -Wduplicated-branches -fPIC -rdynamic -std=c++2a
DBG_FLAGS=-D_DEBUG -g -O0
TARGET_NAME=$(LIB_NAME).cnest

x64_DIR = linux_release/x64
x86_DIR = linux_release/x86
DBG_DIR = linux_debug

CLINKS = -lm -ldl -lnest $(LIB_LINKS)
CLINK_DIR_DBG := -L$(DBG_DIR)
CLINK_DIR_x86 := -L$(x86_DIR)
CLINK_DIR_x64 := -L$(x64_DIR)
CLINKS_DBG := $(CLINK_DIR_DBG) $(CLINKS)
CLINKS_x86 := $(CLINK_DIR_x86) $(CLINKS)
CLINKS_x64 := $(CLINK_DIR_x64) $(CLINKS)

SRCS := $(filter-out $(LIB_NAME)/dllmain.cpp, $(wildcard $(LIB_NAME)/*.c*))
HEADERS := $(filter-out $(LIB_NAME)/framework.h, $(wildcard $(LIB_NAME)/*.h*))
DBG_TARGET := $(DBG_DIR)/$(TARGET_NAME)
x64_TARGET := $(x64_DIR)/$(TARGET_NAME)
x86_TARGET := $(x86_DIR)/$(TARGET_NAME)

.PHONY: debug x86

$(x64_TARGET): $(SRCS) $(HEADERS)
	@mkdir -p $(x64_DIR)
	@echo "Compiling $(TARGET_NAME) for x64..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x64) -O3 -o $(x64_TARGET)

x86: $(x86_TARGET);
$(x86_TARGET): $(SRCS) $(HEADERS)
	@mkdir -p $(x86_DIR)
	@echo "Compiling $(TARGET_NAME) for x86..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_x86) -O3 -m32 -o $(x86_TARGET)

debug: $(DBG_TARGET);
$(DBG_TARGET): $(SRCS) $(HEADERS)
	@mkdir -p $(DBG_DIR)
	@echo "Compiling $(TARGET_NAME) in debug mode..."
	@$(CC) $(CFLAGS) $(SRCS) $(CLINKS_DBG) $(DBG_FLAGS) -o $(DBG_TARGET)
