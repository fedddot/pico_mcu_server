#ifndef	PICO_STEPPER_MOTOR_HPP
#define	PICO_STEPPER_MOTOR_HPP

#include <stdexcept>

#include "stepper_motor.hpp"

namespace pico_mcu_platform {

	class PicoStepperMotor: public manager::StepperMotor {
	public:
		PicoStepperMotor() = default;
		PicoStepperMotor(const PicoStepperMotor& other) = default;
		PicoStepperMotor& operator=(const PicoStepperMotor& other) = delete;
		void steps(const Direction& direction, unsigned int steps_num, unsigned int step_duration_ms) override;
		manager::StepperMotor *clone() const override;
	};

	inline void PicoStepperMotor::steps(const Direction& direction, unsigned int steps_num, unsigned int step_duration_ms) {
		throw std::runtime_error("NOT_IMPLEMENTED");
	}

	inline manager::StepperMotor *PicoStepperMotor::clone() const {
		return new PicoStepperMotor(*this);
	}
}

#endif // PICO_STEPPER_MOTOR_HPP