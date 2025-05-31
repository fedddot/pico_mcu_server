#include "gtest/gtest.h"
#include "pb.h"
#include "pb_encode.h"

#include "ipc_data.hpp"
#include "linear_movement_request.hpp"
#include "movement_manager_data.hpp"
#include "movement_proto_api_request_parser.hpp"
#include "movement_vendor_api.pb.h"
#include "movement_manager_vector.hpp"

using namespace ipc;
using namespace vendor;
using namespace manager;

static RawData serialize_linear_movement_request(const LinearMovementRequest& request);

TEST(ut_movement_proto_api_request_parser, sanity) {
	// GIVEN
	const auto request = LinearMovementRequest(
		Vector<double>(1.0, 2.0, 3.0),
		4.0
	);
	const auto raw_data = serialize_linear_movement_request(request);
	
	// WHEN
	const auto instance = MovementProtoApiRequestParser();

	// THEN
	ASSERT_NO_THROW({
		const auto result = instance(raw_data);
		const auto& casted_result = dynamic_cast<const LinearMovementRequest&>(result.get());
		ASSERT_EQ(casted_result.speed(), request.speed());
	});
}

inline RawData serialize_linear_movement_request(const LinearMovementRequest& request) {
	const auto pb_target = movement_vendor_api_Vector {
		.x = static_cast<float>(request.destination().get(Axis::X)),
		.y = static_cast<float>(request.destination().get(Axis::Y)),
		.z = static_cast<float>(request.destination().get(Axis::Z)),
	};
	const auto pb_linear_request = movement_vendor_api_LinearMovementRequest {
		.speed = static_cast<float>(request.speed()),
		.has_target = true,
		.target = pb_target,
	};
	const auto pb_request = movement_vendor_api_MovementApiRequest {
		.which_request = movement_vendor_api_MovementApiRequest_linear_movement_request_tag,
		.request = {
			.linear_movement_request = pb_linear_request,
		},
	};

	pb_byte_t buffer[256];
	auto ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	if (!pb_encode(&ostream, movement_vendor_api_MovementApiRequest_fields, &pb_request)) {
		throw std::runtime_error("Failed to encode LinearMovementRequest to raw data");
	}
	return RawData((const char *)buffer, (const char *)(buffer + ostream.bytes_written));
}