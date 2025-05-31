add_executable(
	movement_proto_tests
	${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ut_movement_proto_api_request_parser.cpp
)

target_link_libraries(
	movement_proto_tests
	PRIVATE
    movement_proto_api_request_parser
	linear_movement_request
	pb_encode
	
	gtest
)