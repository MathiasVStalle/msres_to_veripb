CXX ?= g++
NAME := MSConvert

CFLAGS = -std=c++20 -g "-I./lib/VeriPB_Prooflogger/core"
LDFLAGS = -lstdc++ -lm

SRC_DIR ?= ./src
BUILD_DIR ?= ./build

SOURCES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/$(SRC_DIR)/%.o, $(SOURCES))
CFLAGS 	:= $(CFLAGS)
LDFLAGS := $(LDFLAGS)
TARGET 	:= $(NAME)


.PHONY: all
all: ./$(TARGET)

.PHONY: run
run: all
	./$(TARGET)

.PHONY: test
test: all
	./$(TARGET) example/unit_clause/test.wcnf example/unit_clause/test.msres temp.pbp

.PHONY: build
build:
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)
	find $(SRC_DIR) -type d -exec mkdir -p $(BUILD_DIR)/{} \;

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp | build
	$(CXX) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(NAME) $(BUILD_DIR) $(TARGET)