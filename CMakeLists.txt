cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_FETCH_FROM_GIT ON)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)

include(external/pico_sdk_import.cmake)

project(pico_mcu_server C CXX ASM)

pico_sdk_init()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(NANOIPC_TESTS OFF)
set(NANOIPC_LINUX_FEATURES OFF)
include(FetchContent)
FetchContent_Declare(
    nanoipc
    GIT_REPOSITORY https://github.com/fedddot/nanoipc.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(nanoipc)
FetchContent_Declare(
    mpu6050
    GIT_REPOSITORY https://github.com/libdriver/mpu6050.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(mpu6050)

add_library(driver_mpu6050_pico INTERFACE)
target_sources(driver_mpu6050_pico INTERFACE ${mpu6050_SOURCE_DIR}/src/driver_mpu6050.c)
target_include_directories(
    driver_mpu6050_pico
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${mpu6050_SOURCE_DIR}/src
)
target_link_libraries(
    driver_mpu6050_pico
    INTERFACE
    pico_stdlib
    hardware_i2c
)

add_subdirectory(sd_spi)
add_subdirectory(fs)

add_executable(pico_mcu_server ${CMAKE_CURRENT_LIST_DIR}/src/pico_mcu_server.cpp)
target_include_directories(pico_mcu_server PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_mcu_server
    PRIVATE
    cobs_frame_reader
    cobs_frame_writer
    json_message_reader
    json_message_writer
    ring_buffer
    driver_mpu6050_pico
    pico_stdlib
    ff
)

target_compile_definitions(
    pico_mcu_server
    PRIVATE
    -DBUFF_SIZE=512UL
    -DPICO_IPC_BAUD=9600UL
)