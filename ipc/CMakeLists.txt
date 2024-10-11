add_library(pico_ipc_connection INTERFACE)
target_sources(pico_ipc_connection INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/pico_ipc_connection.cpp)
target_include_directories(pico_ipc_connection INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_ipc_connection
    INTERFACE
    buffered_ipc_connection
    ipc_connection
    request
    response
    pico_stdlib
)

add_library(pico_synchronous_ipc_connection INTERFACE)
target_sources(pico_synchronous_ipc_connection INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/pico_synchronous_ipc_connection.cpp)
target_include_directories(pico_synchronous_ipc_connection INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_synchronous_ipc_connection
    INTERFACE
    buffered_ipc_connection
    ipc_connection
    request
    response
    pico_stdlib
)

add_subdirectory(tests EXCLUDE_FROM_ALL)