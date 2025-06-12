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

add_library(axis_config INTERFACE)
target_include_directories(axis_config INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    axis_config
    INTERFACE
    pico_stdlib
    movement_manager_data
    pico_stepper_motor
)

add_library(pico_axis_controller_config INTERFACE)
target_include_directories(pico_axis_controller_config INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_axis_controller_config
    INTERFACE
    axis_config
    movement_manager_data
)

add_library(pico_stepper_motor INTERFACE)
target_include_directories(pico_stepper_motor INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_stepper_motor INTERFACE pico_stdlib)

add_library(pico_axis_controller INTERFACE)
target_include_directories(pico_axis_controller INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_axis_controller
    INTERFACE
    axes_controller
    manager_instance
    movement_manager_data
    pico_axis_controller_config
    pico_stepper_motor
    pico_stdlib
)

add_executable(pico_mcu_server ${CMAKE_CURRENT_LIST_DIR}/src/pico_mcu_server.cpp)
target_include_directories(pico_mcu_server PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_mcu_server
    PRIVATE
    pico_stdlib
    hardware_pwm
)

add_subdirectory(proto)
add_subdirectory(external)