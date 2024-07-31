add_executable(
	pico_platform_tests
	${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ut_pico_gpi.cpp
)

target_link_libraries(
	pico_platform_tests
	PRIVATE
    pico_gpi

	gtest
)