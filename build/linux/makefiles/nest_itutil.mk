LIB_NAME = itutil

CC = g++
CFLAGS = -I../../../include -Wall -shared -fPIC -rdynamic -std=c++2a
DBG_FLAGS = -D_DEBUG -g -O0
OUT_FILE = std$(LIB_NAME).so

x64_DIR = ../linux_release/x64
x86_DIR = ../linux_release/x86
DBG_DIR = ../linux_debug

CLINKS = -lm -ldl -L$(DBG_DIR) -L$(x64_DIR) -L$(x86_DIR) -lnest

SRCS := $(wildcard ../../../libs/nest_$(LIB_NAME)/*.cpp)
DBG_TARGET := $(DBG_DIR)/$(OUT_FILE)
x64_TARGET := $(x64_DIR)/$(OUT_FILE)
x86_TARGET := $(x86_DIR)/$(OUT_FILE)
NEST_LIB_DBG := $(DBG_DIR)/libnest.so
NEST_LIB_x64 := $(x64_DIR)/libnest.so
NEST_LIB_x86 := $(x86_DIR)/libnest.so

.PHONY: debug x86

$(x64_TARGET): $(SRCS) $(NEST_LIB_x64)
	mkdir -p $(x64_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -o $(x64_TARGET)

x86: $(SRCS) $(NEST_LIB_x86)
	mkdir -p $(x86_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) -O3 -m32 -o $(x86_TARGET)

debug: $(SRCS) $(NEST_LIB_DGB)
	mkdir -p $(DBG_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(CLINKS) $(DBG_FLAGS) -o $(DBG_TARGET)

$(NEST_LIB_x64):
	make -f libnest.mk

$(NEST_LIB_x86):
	make -f libnest.mk x86

$(NEST_LIB_DBG):
	make -f libnest.mk debug
