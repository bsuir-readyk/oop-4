cmake_minimum_required(VERSION 3.16)
project(oop_vehicles LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(PROJECT_INCLUDE_DIRS
    src
    src/vehicles
    src/components
    src/serialization
    src/ui
    src/patterns
    src/plugins
)

add_library(oop_vehicle_core SHARED
    src/components/engine.cpp
    src/components/driver.cpp
    src/components/fleet.cpp
    src/serialization/vehicle_serializer.cpp
    src/vehicles/vehicle.cpp
    src/vehicles/land_vehicle.cpp
    src/vehicles/water_vehicle.cpp
    src/vehicles/car.cpp
    src/vehicles/truck.cpp
    src/vehicles/boat.cpp
    src/vehicles/submarine.cpp
    src/patterns/vehicle_factory.cpp
    src/plugins/vehicle_plugin_api.h
    src/plugins/vehicle_type_registry.cpp
    src/plugins/plugin_manager.cpp
)

target_include_directories(oop_vehicle_core PUBLIC ${PROJECT_INCLUDE_DIRS})
target_link_libraries(oop_vehicle_core PUBLIC Qt6::Widgets)

if(WIN32)
    set_target_properties(oop_vehicle_core PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_executable(oop_vehicles
    src/main.cpp
    src/ui/main_window.cpp
    src/ui/main_window.h
)

target_link_libraries(oop_vehicles PRIVATE oop_vehicle_core Qt6::Widgets)

add_library(hovercraft_plugin MODULE
    src/plugin_examples/hovercraft_plugin.cpp
    src/plugin_examples/hovercraft_plugin.h
)

add_library(electric_bus_plugin MODULE
    src/plugin_examples/electric_bus_plugin.cpp
    src/plugin_examples/electric_bus_plugin.h
)

target_include_directories(hovercraft_plugin PRIVATE ${PROJECT_INCLUDE_DIRS})
target_link_libraries(hovercraft_plugin PRIVATE oop_vehicle_core Qt6::Widgets)
set_target_properties(hovercraft_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

target_include_directories(electric_bus_plugin PRIVATE ${PROJECT_INCLUDE_DIRS})
target_link_libraries(electric_bus_plugin PRIVATE oop_vehicle_core Qt6::Widgets)
set_target_properties(electric_bus_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

if(APPLE)
    set_target_properties(hovercraft_plugin PROPERTIES
        INSTALL_RPATH "@loader_path/.."
        BUILD_WITH_INSTALL_RPATH ON
    )
    set_target_properties(electric_bus_plugin PROPERTIES
        INSTALL_RPATH "@loader_path/.."
        BUILD_WITH_INSTALL_RPATH ON
    )
endif()
