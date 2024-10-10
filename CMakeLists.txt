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

add_compile_options(
    -I/usr/lib/arm-none-eabi/include/c++/10.3.1
    -I/usr/lib/arm-none-eabi/include/c++/10.3.1/arm-none-eabi/thumb/v6-m/nofp
    -I/usr/lib/arm-none-eabi/include/c++/10.3.1/backward
    -I/usr/lib/gcc/arm-none-eabi/10.3.1/include
    -I/usr/lib/gcc/arm-none-eabi/10.3.1/include-fixed
    -I/usr/lib/arm-none-eabi/include
)

add_executable(pico_mcu_server ${CMAKE_CURRENT_LIST_DIR}/src/pico_mcu_server.cpp)
target_include_directories(pico_mcu_server PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_mcu_server
    PRIVATE
    data_lib
    gpio_manager
    json_data_serializer
    json_request_parser
    json_response_serializer
    pico_gpi
    pico_gpo
    pico_ipc_connection
    stepper_motor
    pico_stepper_motor
    server
    vendor
    gpio

    pico_stdlib
)
target_compile_definitions(
    pico_mcu_server
    PRIVATE
    -DMSG_HEAD="MSG_HEAD"
    -DMSG_TAIL="MSG_TAIL"
    -DPICO_IPC_BAUD=115200UL
    -DPICO_IPC_MAX_BUFF_SIZE=1000UL
)

add_subdirectory(external)

add_subdirectory(ipc)
add_subdirectory(platform)