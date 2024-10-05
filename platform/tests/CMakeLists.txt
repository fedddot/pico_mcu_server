add_executable(pico_mcu_platform_tests ${CMAKE_CURRENT_LIST_DIR}/src/ut_pico_mcu_platform.cpp)
target_link_libraries(
	pico_mcu_platform_tests
	PRIVATE
    pico_gpi
	pico_gpo
	pico_stepper_motor
	gpio
)