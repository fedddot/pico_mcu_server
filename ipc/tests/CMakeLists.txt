add_executable(pico_ipc_connection_tests ${CMAKE_CURRENT_LIST_DIR}/src/ut_pico_ipc_connection.cpp)
target_link_libraries(pico_ipc_connection_tests PRIVATE pico_ipc_connection)

# set(TARGET_DIR ${CMAKE_BINARY_DIR}/bin)
get_property(TARGET_DIR TARGET pico_ipc_connection_tests PROPERTY RUNTIME_OUTPUT_DIRECTORY)
set(TARGET_PATH ${TARGET_DIR}/pico_ipc_connection_tests.elf)
get_property(OPENOCD_WORKDIR TARGET build_openocd PROPERTY OPENOCD_RUN_PATH)
get_property(OPENOCD_EXEC TARGET build_openocd PROPERTY OPENOCD_EXEC_PATH)

add_custom_target(
    upload_pico_ipc_connection_tests_to_mcu
    COMMENT "Uploading ${TARGET_PATH} to pico board"
    COMMAND sudo ${OPENOCD_EXEC} -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 1000" -c "program ${TARGET_PATH} verify reset exit"
    DEPENDS build_openocd pico_ipc_connection_tests
    WORKING_DIRECTORY ${OPENOCD_WORKDIR}
)