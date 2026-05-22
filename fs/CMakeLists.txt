add_subdirectory(sd_spi)

add_library(sd_utils INTERFACE)
target_sources(sd_utils INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/sd_utils.cpp)
target_include_directories(sd_utils INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(sd_utils INTERFACE sd_io_pico)

FetchContent_Declare(
    fatfs
    GIT_REPOSITORY  https://github.com/abbrev/fatfs.git
    GIT_TAG         R0.16
)
FetchContent_MakeAvailable(fatfs)

add_library(ff INTERFACE)
target_sources(ff INTERFACE ${fatfs_SOURCE_DIR}/source/ff.c ${CMAKE_CURRENT_LIST_DIR}/src/fatfs_callbacks.cpp)
target_include_directories(ff INTERFACE ${fatfs_SOURCE_DIR}/source)
target_link_libraries(ff INTERFACE sd_utils)
target_compile_definitions(ff INTERFACE FFCONF_DEF=80386)
target_compile_options(
    ff
    INTERFACE
    -include ${CMAKE_CURRENT_LIST_DIR}/include/custom_ffconf.h
)