set(OPENOCD_SRC_URL https://github.com/openocd-org/openocd.git)
set(OPENOCD_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/openocd)
add_custom_command(
    OUTPUT ${OPENOCD_SRC_DIR} ${OPENOCD_SRC_DIR}/bootstrap
    COMMENT "Cloning OpenOCD sources from ${OPENOCD_SRC_URL} into ${OPENOCD_SRC_DIR}"
    COMMAND git clone ${OPENOCD_SRC_URL} openocd
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
    OUTPUT ${OPENOCD_SRC_DIR}/configure
    COMMENT "Creating OpenOCD config"
    COMMAND ./bootstrap
    WORKING_DIRECTORY ${OPENOCD_SRC_DIR}
    DEPENDS ${OPENOCD_SRC_DIR}/bootstrap
)

set(OPENOCD_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/openocd_bin)
add_custom_command(
    OUTPUT ${OPENOCD_BIN_DIR}
    COMMENT "Creating OpenOCD bin dir ${OPENOCD_BIN_DIR}"
    COMMAND mkdir -p ${OPENOCD_BIN_DIR}
)

add_custom_target(
    install_hidapi
    COMMENT "Installing libhidapi-dev"
    COMMAND sudo apt-get update && sudo apt-get install -y libhidapi-dev
)

add_custom_command(
    OUTPUT ${OPENOCD_BIN_DIR}/Makefile
    COMMENT "Config OpenOCD build"
    COMMAND ${OPENOCD_SRC_DIR}/configure --enable-cmsis-dap-v2 --enable-cmsis-dap
    WORKING_DIRECTORY ${OPENOCD_BIN_DIR}
    DEPENDS ${OPENOCD_BIN_DIR} ${OPENOCD_SRC_DIR}/configure install_hidapi
)

# lines 67-68
set(CMSIS_DAP_DST_PATH ${OPENOCD_SRC_DIR}/src/jtag/drivers/cmsis_dap.c)
set(CMSIS_DAP_SRC_PATH ${CMAKE_CURRENT_LIST_DIR}/src/cmsis_dap.c)
add_custom_command(
    OUTPUT ${OPENOCD_BIN_DIR}/src/openocd
    COMMENT "building OpenOCD under ${OPENOCD_BIN_DIR}"
    COMMAND cp -f ${CMSIS_DAP_SRC_PATH} ${CMSIS_DAP_DST_PATH}
    COMMAND make -j4
    WORKING_DIRECTORY ${OPENOCD_BIN_DIR}
    DEPENDS ${OPENOCD_BIN_DIR}/Makefile
)

set(TARGET_PATH ${CMAKE_BINARY_DIR}/bin/pico_mcu_server.elf)
add_custom_target(
    run_openocd
    COMMENT "Running openocd"
    COMMAND sudo ${OPENOCD_BIN_DIR}/src/openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program ${TARGET_PATH} verify reset exit"
    DEPENDS ${OPENOCD_BIN_DIR}/src/openocd pico_mcu_server
    WORKING_DIRECTORY ${OPENOCD_SRC_DIR}/tcl
)