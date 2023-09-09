LIB_NAME=gui
TARGET=lib$(LIB_NAME).so

SYSROOT_DIR=../../../sysroot
LIBS_DIR=../
BUILD_DIR=.
SRC_DIR=.

CXX=$(SYSROOT_DIR)/bin/x86_64-umbra-g++
CXX_FLAGS=-I$(LIBS_DIR)

SRCS=$(shell find $(SRC_DIR) -type f -name "*.cpp")
OBJS=$(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all install clean

all: $(BUILD_DIR)/$(TARGET) install

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) -shared $^ -o $@

install:
	cp $(TARGET) $(SYSROOT_DIR)/usr/lib/
	-rm -rf $(SYSROOT_DIR)/usr/include/$(LIB_NAME)/
	mkdir $(SYSROOT_DIR)/usr/include/$(LIB_NAME)/
	cp *.hpp $(SYSROOT_DIR)/usr/include/$(LIB_NAME)/

clean:
	rm *.o
	rm *.so
