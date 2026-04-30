cmake_minimum_required(VERSION 3.16)
project(oop_vehicles LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(oop_vehicles
    src/main.cpp
    src/ui/main_window.cpp
    src/ui/main_window.h
    src/components/engine.cpp
    src/components/driver.cpp
    src/components/fleet.cpp
    src/vehicles/vehicle.cpp
    src/vehicles/land_vehicle.cpp
    src/vehicles/water_vehicle.cpp
    src/vehicles/car.cpp
    src/vehicles/truck.cpp
    src/vehicles/boat.cpp
    src/vehicles/submarine.cpp
    src/patterns/vehicle_factory.cpp
)

target_include_directories(oop_vehicles PRIVATE
    src
    src/vehicles
    src/components
    src/ui
    src/patterns
)
target_link_libraries(oop_vehicles PRIVATE Qt6::Widgets)
