add_library(pico_delay INTERFACE)
target_include_directories(pico_delay INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_delay INTERFACE delay pico_stdlib)

add_library(pico_gpi INTERFACE)
target_include_directories(pico_gpi INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_gpi INTERFACE gpi pico_stdlib)

add_library(pico_gpo INTERFACE)
target_include_directories(pico_gpo INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_gpo INTERFACE gpo pico_stdlib)

add_library(pico_mcu_platform INTERFACE)
target_include_directories(pico_mcu_platform INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_mcu_platform
    INTERFACE
    platform
    gpio
    pico_gpi
    pico_gpo
    pico_delay
)

add_library(pico_uart_receiver INTERFACE)
target_include_directories(pico_uart_receiver INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_uart_receiver INTERFACE pico_stdlib message_receiver buffered_message_receiver)

add_library(pico_uart INTERFACE)
target_sources(pico_uart INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/pico_uart.cpp)
target_include_directories(pico_uart INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    pico_uart
    INTERFACE
    pico_stdlib
    buffered_message_receiver
    message_receiver
    message_sender
)

add_subdirectory(tests EXCLUDE_FROM_ALL)