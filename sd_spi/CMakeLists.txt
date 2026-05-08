FetchContent_Declare(
    ulibsd
    GIT_REPOSITORY https://github.com/1nv1/ulibsd.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(ulibsd)

add_library(spi_io_pico INTERFACE)
target_sources(spi_io_pico INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/spi_io.c)
target_include_directories(spi_io_pico INTERFACE ${ulibsd_SOURCE_DIR})

add_library(sd_io_pico INTERFACE)
target_sources(sd_io_pico INTERFACE ${ulibsd_SOURCE_DIR}/sd_io.c)
target_include_directories(sd_io_pico INTERFACE ${ulibsd_SOURCE_DIR})
target_link_libraries(sd_io_pico INTERFACE spi_io_pico)