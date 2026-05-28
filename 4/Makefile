BUILD_DIR := build
TARGET := oop_vehicles
HOVERCRAFT_PLUGIN := hovercraft_plugin
ELECTRIC_BUS_PLUGIN := electric_bus_plugin

.PHONY: all configure build run plugins hovercraft-plugin electric-bus-plugin clean

all: clean configure build

configure:
	@cmake -S . -B $(BUILD_DIR)

build:
	@cmake --build $(BUILD_DIR)

plugins: configure
	@cmake --build $(BUILD_DIR) --target $(HOVERCRAFT_PLUGIN)
	@cmake --build $(BUILD_DIR) --target $(ELECTRIC_BUS_PLUGIN)

hovercraft-plugin: configure
	@cmake --build $(BUILD_DIR) --target $(HOVERCRAFT_PLUGIN)

electric-bus-plugin: configure
	@cmake --build $(BUILD_DIR) --target $(ELECTRIC_BUS_PLUGIN)

run:
	@./$(BUILD_DIR)/$(TARGET)

clean:
	@rm -rf $(BUILD_DIR)
