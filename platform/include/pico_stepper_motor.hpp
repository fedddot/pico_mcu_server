#ifndef	PICO_STEPPER_MOTOR_HPP
#define	PICO_STEPPER_MOTOR_HPP

#include <array>
#include <cstddef>
#include <map>
#include <memory>
#include <stdexcept>

#include "gpio.hpp"
#include "gpo.hpp"
#include "pico_gpo.hpp"
#include "stepper_motor.hpp"

namespace pico_mcu_platform {

	class PicoStepperMotor: public manager::StepperMotor {
	public:
		enum class Shoulder: int {
			LEFT_HIGH,
			LEFT_LOW,
			RIGHT_HIGH,
			RIGHT_LOW
		};
		using ShouldersMapping = std::map<Shoulder, unsigned int>;
		PicoStepperMotor(const ShouldersMapping& shoulders);
		PicoStepperMotor(const PicoStepperMotor& other) = default;
		PicoStepperMotor& operator=(const PicoStepperMotor& other) = delete;
		void steps(const Direction& direction, unsigned int steps_num, unsigned int step_duration_ms) override;
		manager::StepperMotor *clone() const override;
	private:
		using Shoulders = std::map<Shoulder, std::unique_ptr<manager::Gpo>>;
		using GpioState = typename manager::Gpio::State;
		using State = std::map<Shoulder, GpioState>;
		enum: std::size_t {STATES_NUM = 8};
		using States = std::array<State, STATES_NUM>;

		Shoulders m_shoulders;
		static const States s_states;
		std::size_t m_current_state_number;

		static std::size_t get_next_state(const std::size_t& from, const Direction& direction);
		void apply_state(const State& state);
	};

	inline PicoStepperMotor::PicoStepperMotor(const ShouldersMapping& shoulders): m_current_state_number(0UL) {
		for (const auto& [shoulder, gpo_num]: shoulders) {
			m_shoulders.insert({shoulder, std::unique_ptr<manager::Gpo>(new PicoGpo(gpo_num))});
		}
	}

	inline void PicoStepperMotor::steps(const Direction& direction, unsigned int steps_num, unsigned int step_duration_ms) {
		
	}

	inline manager::StepperMotor *PicoStepperMotor::clone() const {
		return new PicoStepperMotor(*this);
	}
}

#endif // PICO_STEPPER_MOTOR_HPP