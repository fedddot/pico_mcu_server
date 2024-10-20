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
			A0,
			A1,
			B0,
			B1
		};
		using ShouldersMapping = std::map<Shoulder, unsigned int>;
		PicoStepperMotor(const ShouldersMapping& shoulders, const unsigned int enable);
		PicoStepperMotor(const PicoStepperMotor& other) = delete;
		PicoStepperMotor& operator=(const PicoStepperMotor& other) = delete;

		void enable() override;
		void disable() override;
		bool enabled() const override;
		void step(const Direction& direction) override;
	private:
		using Shoulders = std::map<Shoulder, std::unique_ptr<manager::Gpo>>;
		using GpioState = typename manager::Gpio::State;
		using State = std::map<Shoulder, GpioState>;
		enum: std::size_t {STATES_NUM = 8};
		using States = std::array<State, STATES_NUM>;

		Shoulders m_shoulders;
		std::unique_ptr<manager::Gpo> m_enable;

		static const States s_states;
		std::size_t m_current_state_number;

		static std::size_t get_next_state(const std::size_t& from, const Direction& direction);
		void apply_state(const State& state);
	};

	inline PicoStepperMotor::PicoStepperMotor(const ShouldersMapping& shoulders, const unsigned int enable): m_current_state_number(0UL), m_enable(new PicoGpo(enable)) {
		for (const auto& [shoulder, gpo_num]: shoulders) {
			m_shoulders.insert({shoulder, std::unique_ptr<manager::Gpo>(new PicoGpo(gpo_num))});
		}
	}

	inline void PicoStepperMotor::enable() {
		m_enable->set_state(GpioState::HIGH);
	}

	inline void PicoStepperMotor::disable() {
		m_enable->set_state(GpioState::LOW);
	}

	inline bool PicoStepperMotor::enabled() const {
		return GpioState::HIGH == m_enable->state();
	}

	inline void PicoStepperMotor::step(const Direction& direction) {
		auto next_state = get_next_state(m_current_state_number, direction);
		apply_state(s_states[next_state]);
		m_current_state_number = next_state;
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
		disable();
		for (const auto& [shoulder, gpo_state]: state) {
			m_shoulders[shoulder]->set_state(gpo_state);
		}
		enable();
	}
}

#endif // PICO_STEPPER_MOTOR_HPP