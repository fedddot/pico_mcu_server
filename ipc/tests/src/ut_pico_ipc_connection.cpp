#include <stdexcept>

#include "pico_ipc_connection.hpp"

using namespace pico_mcu_ipc;

int main(void) {
	// GIVEN
	const int gpi_id(2);
	
	// WHEN
	PicoIpcConnection *instance_ptr(nullptr);

	// THEN
	try {
		instance_ptr = new PicoIpcConnection();
		if (nullptr == instance_ptr) {
			throw std::runtime_error("");
		}
		delete instance_ptr;
		instance_ptr = nullptr;
	} catch (...) {
		return -1;
	}

	return 0;
}
