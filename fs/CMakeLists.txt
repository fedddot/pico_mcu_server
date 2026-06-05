FetchContent_Declare(
    fatfs
    GIT_REPOSITORY  https://github.com/abbrev/fatfs.git
    GIT_TAG         R0.16
)
FetchContent_MakeAvailable(fatfs)

FetchContent_Declare(
    sdspidriver
    GIT_REPOSITORY https://github.com/fedddot/sdspidriver.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(sdspidriver)

add_library(fatfs_callbacks INTERFACE)
target_sources(fatfs_callbacks INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/fatfs_callbacks.cpp)
target_include_directories(fatfs_callbacks INTERFACE ${fatfs_SOURCE_DIR}/source ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(fatfs_callbacks INTERFACE sdspidriver)

add_library(ff INTERFACE)
target_sources(ff INTERFACE ${fatfs_SOURCE_DIR}/source/ff.c)
target_include_directories(ff INTERFACE ${fatfs_SOURCE_DIR}/source)
target_link_libraries(ff INTERFACE fatfs_callbacks)
target_compile_definitions(ff INTERFACE FFCONF_DEF=80386)
target_compile_options(
    ff
    INTERFACE
    -include ${CMAKE_CURRENT_LIST_DIR}/include/custom_ffconf.h
)