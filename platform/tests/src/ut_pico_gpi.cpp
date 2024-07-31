#include <stdexcept>

#include "pico_gpi.hpp"

using namespace pico_mcu_platform;

int main(void) {
	// GIVEN
	const int gpi_id(2);
	
	// WHEN
	PicoGpi *instance_ptr(nullptr);

	// THEN
	try {
		instance_ptr = new PicoGpi(gpi_id);
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
