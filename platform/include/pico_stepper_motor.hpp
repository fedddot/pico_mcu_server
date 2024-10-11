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
		void steps(const Direction& direction, const unsigned int steps_num, const unsigned int step_duration_ms) override;
		manager::StepperMotor *clone() const override;
	private:
		using Shoulders = std::map<Shoulder, std::shared_ptr<manager::Gpo>>;
		using GpioState = typename manager::Gpio::State;
		using State = std::map<Shoulder, GpioState>;
		enum: std::size_t {STATES_NUM = 8};
		using States = std::array<State, STATES_NUM>;

		Shoulders m_shoulders;
		static const States s_states;
		static const State s_shutdown_state;
		std::size_t m_current_state_number;

		static std::size_t get_next_state(const std::size_t& from, const Direction& direction);
		void apply_state(const State& state);
	};

	inline PicoStepperMotor::PicoStepperMotor(const ShouldersMapping& shoulders): m_current_state_number(0UL) {
		for (const auto& [shoulder, gpo_num]: shoulders) {
			m_shoulders.insert({shoulder, std::shared_ptr<manager::Gpo>(new PicoGpo(gpo_num))});
		}
		apply_state(s_shutdown_state);
	}

	inline void PicoStepperMotor::steps(const Direction& direction, const unsigned int steps_num, const unsigned int step_duration_ms) {
		auto steps_to_go(steps_num);
		while (steps_to_go) {
			auto next_state = get_next_state(m_current_state_number, direction);
			apply_state(s_states[next_state]);
			m_current_state_number = next_state;
			--steps_to_go;
		}
	}

	inline manager::StepperMotor *PicoStepperMotor::clone() const {
		return new PicoStepperMotor(*this);
	}

	inline std::size_t PicoStepperMotor::get_next_state(const std::size_t& from, const Direction& direction) {
		auto get_next_cw = [](const std::size_t& from) {
			auto next(from + 1);
			if (s_states.size() <= next) {
				return std::size_t(0UL);
			}
			return next;
		};
		auto get_next_ccw = [](const std::size_t& from) {
			if (0UL == from) {
				return s_states.size() - 1UL;
			}
			return from - 1UL;
		};
		switch (direction) {
		case Direction::CW:
			return get_next_cw(from);
		case Direction::CCW:
			return get_next_ccw(from);
		default:
			throw std::invalid_argument("invalid direction received");
		}
	}

	inline void PicoStepperMotor::apply_state(const State& state) {
		for (const auto& [shoulder, gpo_state]: state) {
			m_shoulders[shoulder]->set_state(gpo_state);
		}
	}
}

#endif // PICO_STEPPER_MOTOR_HPP