add_executable(pico_gpi_tests ${CMAKE_CURRENT_LIST_DIR}/src/ut_pico_gpi.cpp)
target_link_libraries(
	pico_gpi_tests
	PRIVATE
    pico_gpi
)