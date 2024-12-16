if(NOT DEFINED ENV{MCU_SERVER_PATH})
    message(FATAL_ERROR, "variable MCU_SERVER_PATH is not defined")
endif()
add_subdirectory($ENV{MCU_SERVER_PATH} ${CMAKE_BINARY_DIR}/_deps/mcu_server-build)