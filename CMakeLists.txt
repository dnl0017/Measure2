# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE Debug)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/pi/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Measure2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Measure2 
                DHT.cpp
                lcd.c
                Measure2.cpp 
                )

pico_set_program_name(Measure2 "Measure2")
pico_set_program_version(Measure2 "0.1")

pico_enable_stdio_uart(Measure2 1)
pico_enable_stdio_usb(Measure2 0)

# Add the standard library to the build
target_link_libraries(Measure2 pico_stdlib)

# Add any user requested libraries
target_link_libraries(Measure2
                        pico_stdlib
                        pico_multicore
                        pico_binary_info
                        hardware_timer
                        hardware_gpio
                       )

pico_add_extra_outputs(Measure2)

