LIB_NAME=core

SYSROOT_DIR=../../../sysroot

.PHONY: all install

all: install

install:
	-rm -rf $(SYSROOT_DIR)/usr/include/$(LIB_NAME)/
	mkdir $(SYSROOT_DIR)/usr/include/$(LIB_NAME)/
	cp *.hpp $(SYSROOT_DIR)/usr/include/$(LIB_NAME)/
