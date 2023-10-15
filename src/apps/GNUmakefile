SYSROOT_DIR=../../sysroot
BASE_DIR=../../base
CXX=$(SYSROOT_DIR)/bin/x86_64-umbra-g++
CC=$(SYSROOT_DIR)/bin/x86_64-umbra-gcc

all:
	$(CXX) test.cpp -g -o $(BASE_DIR)/test
	$(CC) ping.c -g -o $(BASE_DIR)/ping
