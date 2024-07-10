set(OPENOCD_SRC_URL https://github.com/openocd-org/openocd.git)
set(OPENOCD_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/openocd)
add_custom_command(
    OUTPUT ${OPENOCD_SRC_DIR}/bootstrap
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
    
)

list(APPEND HIDAPI_PRODUCTS /usr/include/hidapi/hidapi.h /usr/include/hidapi/hidapi_libusb.h)
add_custom_target(
    install_hidapi
    COMMENT "Installing libhidapi-dev"
    COMMAND sudo apt-get update && sudo apt-get install -y libhidapi-dev
)

add_custom_command(
    OUTPUT ${OPENOCD_BIN_DIR}/Makefile
    COMMENT "Config OpenOCD build"
    COMMAND mkdir -p ${OPENOCD_BIN_DIR}
    COMMAND cd ${OPENOCD_BIN_DIR} && ${OPENOCD_SRC_DIR}/configure --enable-cmsis-dap-v2 --enable-cmsis-dap
    DEPENDS ${OPENOCD_SRC_DIR}/configure install_hidapi
)

set(CMSIS_DAP_DIR ${OPENOCD_SRC_DIR}/src/jtag/drivers)
add_custom_command(
    OUTPUT ${OPENOCD_BIN_DIR}/src/openocd
    COMMENT "building OpenOCD under ${OPENOCD_BIN_DIR}"
    COMMAND cp -f ${CMSIS_DAP_DIR}/cmsis_dap.c ${CMAKE_CURRENT_SOURCE_DIR}/src/cmsis_dap_tmp.c
    COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/src/cmsis_dap.c ${CMSIS_DAP_DIR}/cmsis_dap.c
    COMMAND make -j4
    COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/src/cmsis_dap_tmp.c ${CMSIS_DAP_DIR}/cmsis_dap.c
    WORKING_DIRECTORY ${OPENOCD_BIN_DIR}
    DEPENDS ${OPENOCD_BIN_DIR}/Makefile
)

add_custom_target(build_openocd DEPENDS ${OPENOCD_BIN_DIR}/src/openocd)

set_property(TARGET build_openocd PROPERTY OPENOCD_RUN_PATH ${OPENOCD_SRC_DIR}/tcl)
set_property(TARGET build_openocd PROPERTY OPENOCD_EXEC_PATH ${OPENOCD_BIN_DIR}/src/openocd)