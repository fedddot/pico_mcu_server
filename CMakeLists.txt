cmake_minimum_required(VERSION 3.12)

project(pico_mcu_server C CXX ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(pico_mcu_server_types INTERFACE)
target_include_directories(pico_mcu_server_types INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(pico_data_sender INTERFACE)
target_include_directories(pico_data_sender INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_data_sender INTERFACE pico_mcu_server_types)

add_executable(pico_mcu_server ${CMAKE_CURRENT_LIST_DIR}/src/pico_mcu_server.cpp)
target_include_directories(pico_mcu_server PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_mcu_server
    PRIVATE
    mcu_server
    mcu_task_engine
    pico_mcu_server_types
    json_data_parser
    json_data_serializer
    custom_retriever
    data_lib
)
add_subdirectory(external)