add_library(pico_gpi INTERFACE)
target_include_directories(pico_gpi INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_gpi INTERFACE gpi pico_stdlib)

add_library(pico_gpo INTERFACE)
target_include_directories(pico_gpo INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_gpo INTERFACE gpo pico_stdlib)



add_library(pico_uart_receiver INTERFACE)
target_include_directories(pico_uart_receiver INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_uart_receiver INTERFACE pico_stdlib message_receiver)

add_library(pico_uart INTERFACE)
target_include_directories(pico_uart INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_uart INTERFACE gpo pico_stdlib listener)

add_library(pico_delay INTERFACE)
target_include_directories(pico_delay INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_delay INTERFACE delay pico_stdlib)

add_library(pico_data_sender INTERFACE)
target_include_directories(pico_data_sender INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_data_sender INTERFACE pico_uart pico_mcu_server_types)

add_subdirectory(tests EXCLUDE_FROM_ALL)