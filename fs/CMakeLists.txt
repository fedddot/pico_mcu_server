FetchContent_Declare(
    fatfs
    GIT_REPOSITORY  https://github.com/abbrev/fatfs.git
    GIT_TAG         R0.16
)
FetchContent_MakeAvailable(fatfs)

add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/fs_generated/disk.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/fs_generated && xxd -i disk.img | sed "s/^unsigned char/const unsigned char/" > ${CMAKE_BINARY_DIR}/fs_generated/disk.c
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/src/disk.img
)

add_custom_target(
	generate_fs_src
	DEPENDS ${CMAKE_BINARY_DIR}/fs_generated/disk.c
)

add_library(ff STATIC ${fatfs_SOURCE_DIR}/source/ff.c ${CMAKE_CURRENT_LIST_DIR}/src/fatfs_callbacks.cpp ${CMAKE_BINARY_DIR}/fs_generated/disk.c)
add_dependencies(ff generate_fs_src)
target_include_directories(ff PUBLIC ${fatfs_SOURCE_DIR}/source)
target_link_libraries(ff PUBLIC hardware_flash hardware_sync)
target_compile_definitions(ff PRIVATE FFCONF_DEF=80386)
target_compile_options(
    ff
    PRIVATE
    -include ${CMAKE_CURRENT_LIST_DIR}/include/custom_ffconf.h
)