CXX ?= g++
NAME := MSConvert

# Flags
WARN_FLAGS = -Wall -Wextra -Werror
CFLAGS_COMMON = -std=c++20 -g
LDFLAGS = -lstdc++ -lm

# Include paths
INCLUDES = -I./lib/VeriPB_Prooflogger/core

# Source directories
SRC_DIR := ./src
SUBMOD_DIR := ./lib/VeriPB_Prooflogger
BUILD_DIR := ./build
TEST_DIR := ./example/test

# Source files
SRC_SOURCES := $(shell find $(SRC_DIR) -name '*.cpp')
SUBMOD_SOURCES := $(shell find $(SUBMOD_DIR) -name '*.cpp')

# Object files
SRC_OBJECTS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRC_SOURCES))
SUBMOD_OBJECTS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SUBMOD_SOURCES))
OBJECTS := $(SRC_OBJECTS) $(SUBMOD_OBJECTS)

TARGET := $(NAME)

# Default target
.PHONY: all
all: ./$(TARGET)

# Build target
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile your own code with warnings
$(BUILD_DIR)/%.o: %.cpp | build
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS_COMMON) $(WARN_FLAGS) $(INCLUDES) -c $< -o $@

# Compile submodule code with no warnings
$(BUILD_DIR)/$(SUBMOD_DIR)/%.o: $(SUBMOD_DIR)/%.cpp | build
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS_COMMON) $(INCLUDES) -c $< -o $@

# Ensure build dirs exist
.PHONY: build
build:
	mkdir -p $(BUILD_DIR)

# Test / run
.PHONY: run
run: all
	./$(TARGET)

.PHONY: test
test: all
	./$(TARGET) $(TEST_DIR)/test.wcnf $(TEST_DIR)/test.msres temp.pbp

# Clean
.PHONY: clean
clean:
	rm -rf $(NAME) $(BUILD_DIR) $(TARGET)
