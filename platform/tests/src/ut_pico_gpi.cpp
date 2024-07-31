#include "pico_gpi.hpp"

using namespace pico_mcu_platform;

TEST(ut_pico_gpi, ctor_dtor_sanity) {
	// GIVEN
	const int gpi_id(2);
	
	// WHEN
	PicoGpi *instance_ptr(nullptr);

	// THEN
	ASSERT_NO_THROW(
		instance_ptr = new PicoGpi(gpi_id);
	);
	ASSERT_NO_THROW(delete instance_ptr);
	instance_ptr = nullptr;
}
