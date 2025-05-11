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

add_library(pico_stepper_motor INTERFACE)
target_include_directories(pico_stepper_motor INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_stepper_motor
    INTERFACE
    stepper_motor
    stepper_motor_data
    pico_stdlib
)

add_library(pico_axis_controller INTERFACE)
target_include_directories(pico_axis_controller INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_axis_controller
    INTERFACE
    pico_stepper_motor
    pico_stdlib
)

add_executable(pico_mcu_server ${CMAKE_CURRENT_LIST_DIR}/src/pico_mcu_server.cpp)
target_include_directories(pico_mcu_server PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_mcu_server
    PRIVATE
    axes_controller
    ipc_instance
    manager_instance
    movement_host_builder
    movement_manager_data
    pico_axis_controller
    pico_stepper_motor
    raw_data_package_descriptor
    raw_data_package_reader
    raw_data_package_utils
    raw_data_package_writer
    stepper_motor
    stepper_motor_data
    
    jsoncpp_static
    pico_stdlib
)
target_compile_definitions(
    pico_mcu_server
    PRIVATE
    -DMSG_PREAMBLE="MSG_PREAMBLE"
    -DMSG_SIZE_FIELD_LEN=4UL
    -DPICO_IPC_BAUD=115200UL
)

add_subdirectory(external)