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
    pico_stdlib
    mcu_server
    mcu_task_engine
    pico_mcu_server_types
    json_data_parser
    json_data_serializer
    custom_retriever
    data_lib
)
target_compile_definitions(
    pico_mcu_server
    PRIVATE
    -DMSG_HEADER="MSG_HEADER"
    -DMSG_TAIL="MSG_TAIL"
)

add_subdirectory(external)

get_property(TARGET_DIR TARGET pico_mcu_server PROPERTY RUNTIME_OUTPUT_DIRECTORY)
get_property(OPENOCD_WORKDIR TARGET build_openocd PROPERTY OPENOCD_RUN_PATH)
get_property(OPENOCD_EXEC TARGET build_openocd PROPERTY OPENOCD_EXEC_PATH)

add_custom_target(
    run_openocd
    COMMENT "Running openocd"
    COMMAND sudo ${OPENOCD_EXEC} -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program ${TARGET_DIR}/pico_mcu_server.elf verify reset exit"
    DEPENDS build_openocd pico_mcu_server
    WORKING_DIRECTORY ${OPENOCD_WORKDIR}
)



