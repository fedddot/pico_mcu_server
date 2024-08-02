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
    custom_creator
    custom_parser
    gpio
    json_data_parser
    json_data_serializer
    mcu_factory
    mcu_server
    pico_ipc_connection
    pico_mcu_platform

    pico_stdlib
)

target_compile_definitions(
    pico_mcu_server
    PRIVATE
    -DMSG_HEADER="MSG_HEADER"
    -DMSG_TAIL="MSG_TAIL"
    -DPICO_IPC_BAUD=9600UL
    -DPICO_IPC_MAX_BUFF_SIZE=1000UL
)

target_compile_options(
    pico_mcu_server
    PRIVATE
    -g
    -O0
)

add_subdirectory(external)

# set(TARGET_DIR ${CMAKE_BINARY_DIR}/bin)
get_property(TARGET_DIR TARGET pico_mcu_server PROPERTY RUNTIME_OUTPUT_DIRECTORY)
set(TARGET_PATH ${TARGET_DIR}/pico_mcu_server.elf)
get_property(OPENOCD_WORKDIR TARGET build_openocd PROPERTY OPENOCD_RUN_PATH)
get_property(OPENOCD_EXEC TARGET build_openocd PROPERTY OPENOCD_EXEC_PATH)

add_custom_target(
    upload_to_mcu
    COMMENT "Uploading ${TARGET_PATH} to pico board"
    COMMAND sudo ${OPENOCD_EXEC} -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 1000" -c "program ${TARGET_PATH} verify reset exit"
    DEPENDS build_openocd pico_mcu_server
    WORKING_DIRECTORY ${OPENOCD_WORKDIR}
)

add_custom_target(
    run_gdb_server
    COMMENT "Running openocd gdb server"
    COMMAND sudo ${OPENOCD_EXEC} -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 1000"
    DEPENDS build_openocd
    WORKING_DIRECTORY ${OPENOCD_WORKDIR}
)

add_subdirectory(ipc)
add_subdirectory(platform)