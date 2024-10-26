#include <stdexcept>

#include "pico_gpi.hpp"
#include "pico_gpo.hpp"

using namespace pico_mcu_platform;

template <typename T>
inline void assert_eq(const T& one, const T& other) {
	if (one != other) {
		throw std::runtime_error("equality assertion failed");
	}
}

template <typename T>
inline void assert_ne(const T& one, const T& other) {
	if (one == other) {
		throw std::runtime_error("inequality assertion failed");
	}
}

int main(void) {
	// GIVEN
	const int gpi_id(2);
	const int gpo_id(2);
	
	// WHEN
	PicoGpi *gpi_ptr(nullptr);
	PicoGpo *gpo_ptr(nullptr);

	// THEN
	try {
		gpi_ptr = new PicoGpi(gpi_id);
		assert_ne(static_cast<PicoGpi *>(nullptr), gpi_ptr);
		assert_eq(PicoGpi::Direction::IN, gpi_ptr->direction());
		assert_eq(PicoGpi::State::LOW, gpi_ptr->state());
		delete gpi_ptr;
		gpi_ptr = nullptr;
	} catch (...) {
		return -1;
	}

	return 0;
}
