set(OPENOCD_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/code)

set(OPENOCD_SRC_URL "https://git.code.sf.net/p/openocd/code")
add_custom_command(
    OUTPUT ${OPENOCD_SRC_DIR} ${OPENOCD_SRC_DIR}/bootstrap
    COMMENT "Fetching OpenOCD sources from ${OPENOCD_SRC_URL} into ${OPENOCD_SRC_DIR}"
    COMMAND git clone ${OPENOCD_SRC_URL}
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

add_custom_command(
    OUTPUT ${OPENOCD_BIN_DIR}/Makefile
    COMMENT "Config OpenOCD build"
    COMMAND ${OPENOCD_SRC_DIR}/configure --enable-ftdi --enable-sysfsgpio --enable-bcm2835gpio
    WORKING_DIRECTORY ${OPENOCD_BIN_DIR}
    DEPENDS ${OPENOCD_BIN_DIR} ${OPENOCD_SRC_DIR}/configure
)

add_custom_target(config_openocd_build DEPENDS ${OPENOCD_BIN_DIR}/Makefile)

add_custom_command(
    OUTPUT ${OPENOCD_BIN_DIR}/src/openocd
    COMMENT "building OpenOCD under ${OPENOCD_BIN_DIR}"
    COMMAND make -j4
    WORKING_DIRECTORY ${OPENOCD_BIN_DIR}
    DEPENDS ${OPENOCD_BIN_DIR}/Makefile
)

# add_custom_command(
#     OUTPUT ${OPENOCD_BIN_DIR}/install/openocd
#     COMMENT "installing OpenOCD"
#     COMMAND sudo make install 
#     WORKING_DIRECTORY ${OPENOCD_BIN_DIR}
#     DEPENDS ${OPENOCD_BIN_DIR}/src/openocd
# )

add_custom_target(build_openocd DEPENDS ${OPENOCD_BIN_DIR}/src/openocd)

add_custom_target(
    run_openocd
    COMMENT "Running openocd"
    COMMAND ${OPENOCD_BIN_DIR}/src/openocd -f ${OPENOCD_SRC_DIR}/tcl/target/rp2040.cfg -s ${OPENOCD_SRC_DIR} -c "transport select hla_swd"
    DEPENDS build_openocd
)