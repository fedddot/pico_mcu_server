FetchContent_Declare(
    fatfs
    GIT_REPOSITORY  https://github.com/abbrev/fatfs.git
    GIT_TAG         R0.16
)
FetchContent_MakeAvailable(fatfs)

set(FS_BIN_PATH ${CMAKE_CURRENT_LIST_DIR}/src/disk.img)
set(GENERATED_FS_SRC_DIR ${CMAKE_BINARY_DIR}/fs_generated)
add_custom_command(
	OUTPUT ${GENERATED_FS_SRC_DIR}/disk.c
	COMMAND mkdir -p ${GENERATED_FS_SRC_DIR} && xxd -i ${FS_BIN_PATH} > ${GENERATED_FS_SRC_DIR}/disk.c
	DEPENDS ${FS_BIN_PATH}
)

add_custom_target(
	generate_fs_src
	DEPENDS ${GENERATED_FS_SRC_DIR}/disk.c
)

add_library(ff STATIC ${fatfs_SOURCE_DIR}/source/ff.c ${CMAKE_CURRENT_LIST_DIR}/src/fatfs_callbacks.cpp ${GENERATED_FS_SRC_DIR}/disk.c)
add_dependencies(ff generate_fs_src)
target_include_directories(ff PUBLIC ${fatfs_SOURCE_DIR}/source)
target_compile_definitions(ff PRIVATE FFCONF_DEF=80386)
target_compile_options(
    ff
    PRIVATE
    -include ${CMAKE_CURRENT_LIST_DIR}/include/custom_ffconf.h
)