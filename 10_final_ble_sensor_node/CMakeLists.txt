cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ble_sensor_node)

set(CMAKE_CXX_STANDARD 17)

target_sources(app PRIVATE
    src/main.cpp
    src/led.cpp
    src/sensor.cpp
    src/ble_manager.cpp
    src/sensor_thread.cpp
)

target_include_directories(app PRIVATE src)
