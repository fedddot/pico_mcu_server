cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_FETCH_FROM_GIT ON)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)
set(PICO_DEOPTIMIZED_DEBUG 1)

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

add_subdirectory(sd_spi_driver)
add_subdirectory(fs)

add_executable(pico_mcu_server ${CMAKE_CURRENT_LIST_DIR}/src/pico_mcu_server.cpp)
target_include_directories(pico_mcu_server PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_mcu_server
    PRIVATE
    pico_stdlib
    ff
    sd_spi_driver
    hardware_spi
    hardware_gpio
    pico_time
)

target_compile_definitions(
    pico_mcu_server
    PRIVATE
    -DBUFF_SIZE=512UL
    -DPICO_IPC_BAUD=9600UL
)

target_compile_options(pico_mcu_server PRIVATE -O0)