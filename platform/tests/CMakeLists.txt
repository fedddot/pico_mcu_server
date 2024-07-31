add_executable(pico_gpi_tests ${CMAKE_CURRENT_LIST_DIR}/src/ut_pico_gpi.cpp)
target_link_libraries(
	pico_gpi_tests
	PRIVATE
    pico_gpi
)

add_executable(pico_uart_receiver_tests ${CMAKE_CURRENT_LIST_DIR}/src/ut_pico_uart_receiver.cpp)
target_link_libraries(
	pico_uart_receiver_tests
	PRIVATE
    pico_uart_receiver
)