#include <cstddef>

#include "pico_ipc_connection.hpp"

using namespace pico_mcu_ipc;

int main(void) {
	// GIVEN
	const PicoIpcConnection::Baud baud(PicoIpcConnection::Baud::B9600);
	const pico_mcu_ipc::PicoIpcData head("MSG_HEADER");
	const pico_mcu_ipc::PicoIpcData data("test_data");
	const pico_mcu_ipc::PicoIpcData tail("MSG_TAIL");
	const std::size_t max_size(1000UL);

	// WHEN
	PicoIpcConnection *instance_ptr(nullptr);

	// THEN
	try {
		PicoIpcConnection instance(baud, head, tail, max_size);
		while (true) {
			if (!instance.readable()) {
				continue;
			}
			auto incomming_data = instance.read();
			instance.send(incomming_data);
		}


	} catch (...) {
		return -1;
	}

	return 0;
}
