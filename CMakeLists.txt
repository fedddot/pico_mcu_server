cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_FETCH_FROM_GIT ON)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)

include(external/pico_sdk_import.cmake)

project(pico_mcu_server C CXX ASM)

pico_sdk_init()

set(CMAKE_CXX_STANDARD 21)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(pico_mcu_server ${CMAKE_CURRENT_LIST_DIR}/src/pico_mcu_server.cpp)
target_include_directories(pico_mcu_server PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_mcu_server
    PRIVATE
    raw_data_package_descriptor
    raw_data_package_reader
    raw_data_package_utils
    raw_data_package_writer
    stepper_host
    stepper_ipc_data_infra
    stepper_motor
    
    pico_stdlib
)
target_compile_definitions(
    pico_mcu_server
    PRIVATE
    -DMSG_PREAMBLE="MSG_HEAD"
    -DMSG_SIZE_FIELD_LEN=8UL
    -DPICO_IPC_BAUD=115200UL
)

add_subdirectory(external)