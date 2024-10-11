add_library(pico_gpi INTERFACE)
target_include_directories(pico_gpi INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_gpi INTERFACE gpi pico_stdlib)

add_library(pico_gpo INTERFACE)
target_include_directories(pico_gpo INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_gpo INTERFACE gpo pico_stdlib)

add_library(pico_stepper_motor INTERFACE)
target_sources(pico_stepper_motor INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/pico_stepper_motor.cpp)
target_include_directories(pico_stepper_motor INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pico_stepper_motor INTERFACE stepper_motor pico_gpo pico_stdlib)

add_subdirectory(tests EXCLUDE_FROM_ALL)