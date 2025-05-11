#ifndef	PICO_STEPPER_MOTOR_HPP
#define	PICO_STEPPER_MOTOR_HPP

#include <cstddef>
#include <stdexcept>

#include "hardware/gpio.h"
#include "pico/time.h"

#include "stepper_motor.hpp"
#include "stepper_motor_data.hpp"

namespace pico {
    class PicoStepper: public manager::StepperMotor {
    public:
        PicoStepper(
            const std::size_t& enable_pin,
            const std::size_t& step_pin,
            const std::size_t& dir_pin,
            const std::size_t& hold_time_us
        );
        PicoStepper(const PicoStepper&) = delete;
        PicoStepper& operator=(const PicoStepper&) = delete;
        ~PicoStepper() noexcept override;
        
        void set_state(const manager::State& state) override;
        manager::State state() const override;    
        void step(const manager::RotationDirection& direction) override;
    private:
        const std::size_t m_enable_pin;
        const std::size_t m_step_pin;
        const std::size_t m_dir_pin;
        const std::size_t m_hold_time_us;
    
        static void init_output(const std::size_t& pin_num, const bool init_value);
    };

    inline PicoStepper::PicoStepper(
        const std::size_t& enable_pin,
        const std::size_t& step_pin,
        const std::size_t& dir_pin,
        const std::size_t& hold_time_us
    ): m_enable_pin(enable_pin),
        m_step_pin(step_pin), m_dir_pin(dir_pin),
        m_hold_time_us(hold_time_us) {
        init_output(m_enable_pin, true); // en pin has inverse logic
        init_output(m_step_pin, false);
        init_output(m_dir_pin, false);
    }

    inline PicoStepper::~PicoStepper() noexcept {
        set_state(manager::State::DISABLED);
        gpio_deinit(m_enable_pin);
        gpio_deinit(m_step_pin);
        gpio_deinit(m_dir_pin);
    }

    inline void PicoStepper::set_state(const manager::State& state) {
        switch (state) {
        case manager::State::ENABLED:
            gpio_put(m_enable_pin, false);
            break;
        case manager::State::DISABLED:
            gpio_put(m_enable_pin, true);
            break;
        default:
            throw std::invalid_argument("unsupported state received");
        }
    }

    inline manager::State PicoStepper::state() const {
        return gpio_get(m_enable_pin) ? manager::State::ENABLED : manager::State::DISABLED;
    }

    inline void PicoStepper::step(const manager::RotationDirection& direction) {
        switch (direction) {
        case manager::RotationDirection::CCW:
            gpio_put(m_dir_pin, true);
            break;
        case manager::RotationDirection::CW:
            gpio_put(m_dir_pin, false);
            break;
        default:
            throw std::invalid_argument("unsupported direction");
        }
        gpio_put(m_step_pin, true);
        sleep_us(m_hold_time_us);
        gpio_put(m_step_pin, false);
        sleep_us(m_hold_time_us);
    }

    inline void PicoStepper::init_output(const std::size_t& pin_num, const bool init_value) {
        gpio_init(pin_num);
        gpio_set_dir(pin_num, GPIO_OUT);
        gpio_put(pin_num, init_value);
    }
}

#endif // PICO_STEPPER_MOTOR_HPP