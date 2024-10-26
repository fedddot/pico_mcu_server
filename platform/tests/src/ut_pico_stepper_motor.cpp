#include <stdexcept>

#include "pico_stepper_motor.hpp"

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
	PicoStepperMotor *motor_ptr(nullptr);
	const PicoStepperMotor::ShouldersMapping shoulders {
		{ PicoStepperMotor::Shoulder::A1,		10 },
		{ PicoStepperMotor::Shoulder::A0,	11 },
		{ PicoStepperMotor::Shoulder::B1,	12 },
		{ PicoStepperMotor::Shoulder::B0,	13 }
	};

	// THEN
	try {
		motor_ptr = new PicoStepperMotor(shoulders);
		assert_ne(static_cast<PicoStepperMotor *>(nullptr), motor_ptr);
		delete motor_ptr;
		motor_ptr = nullptr;
	} catch (...) {
		return -1;
	}

	return 0;
}
