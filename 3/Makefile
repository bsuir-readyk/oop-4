BUILD_DIR := build
TARGET := oop_vehicles

.PHONY: all configure build run clean

all: clean configure build

configure:
	@cmake -S . -B $(BUILD_DIR)

build:
	@cmake --build $(BUILD_DIR)

run:
	@./$(BUILD_DIR)/$(TARGET)

clean:
	@rm -rf $(BUILD_DIR)
