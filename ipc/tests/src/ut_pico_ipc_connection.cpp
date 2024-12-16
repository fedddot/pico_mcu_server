#include <stdexcept>
#include <string>

#include "pico_ipc_connection.hpp"
#include "json_request_parser.hpp"
#include "json_response_serializer.hpp"
#include "request.hpp"
#include "response.hpp"

using namespace pico_mcu_ipc;
using namespace server_utl;

int main(void) {
	// GIVEN
	const PicoIpcConnection::Baud baud(PicoIpcConnection::Baud::B9600);
	const RawData head("head");
	const RawData tail("tail");

	auto matcher = [head, tail](const RawData& data) {
		auto head_pos = data.find(head);
		if (RawData::npos == head_pos) {
			return false;
		}
		auto tail_pos = data.find(tail, head_pos + head.size());
		if (RawData::npos == tail_pos) {
			return false;
		}
		return true;
	};
	auto extractor = [head, tail](RawData *data) {
		auto head_pos = data->find(head);
		if (RawData::npos == head_pos) {
			throw std::invalid_argument("missing head");
		}
		auto tail_pos = data->find(tail, head_pos + head.size());
		if (RawData::npos == tail_pos) {
			throw std::invalid_argument("missing tail");
		}
		RawData extracted(data->begin() + head_pos + head.size(), data->begin() + tail_pos);
		data->erase(data->begin() + head_pos, data->begin() + tail_pos + tail.size());
		return JsonRequestParser()(extracted);
	};
	auto serializer = [head, tail](const server::Response& response) {
		return head + JsonResponseSerializer()(response) + tail;
	};

	// WHEN
	PicoIpcConnection *instance_ptr(nullptr);

	// THEN
	try {
		PicoIpcConnection instance(baud, serializer, matcher, extractor);
		instance.subscribe(
			"test_subsc",
			[&instance](const server::Request& req) {
				instance.send(server::Response(server::Response::ResponseCode::OK, req.body()));
			}
		);

		while (true) {
			
		}

	} catch (...) {
		return -1;
	}

	return 0;
}
