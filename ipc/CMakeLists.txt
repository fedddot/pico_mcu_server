add_library(pico_ipc_connection INTERFACE)
target_sources(pico_ipc_connection INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/pico_ipc_connection.cpp)
target_include_directories(pico_ipc_connection INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_ipc_connection INTERFACE ipc_connection buffered_custom_ipc_connection pico_stdlib)

add_subdirectory(tests EXCLUDE_FROM_ALL)