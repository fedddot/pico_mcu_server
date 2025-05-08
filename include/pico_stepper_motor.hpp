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
        enum Mode {
            FULL_STEP = 0,
            HALF_STEP,
            QUARTER_STEP,
        };
        PicoStepper(
            const std::size_t& enable_pin,
            const std::size_t& step_pin,
            const std::size_t& dir_pin,
            const std::size_t& m0_pin,
            const std::size_t& m1_pin,
            const std::size_t& m2_pin,
            const Mode& mode,
            const std::size_t& hold_time_ms
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
        const std::size_t m_m0_pin;
        const std::size_t m_m1_pin;
        const std::size_t m_m2_pin;
        const std::size_t m_hold_time_ms;
    
        static void init_output(const std::size_t& pin_num, const bool init_value);
        static void set_mode(const Mode& mode, const std::size_t& m0_pin, const std::size_t& m1_pin, const std::size_t& m2_pin);
    };

    inline PicoStepper::PicoStepper(
        const std::size_t& enable_pin,
        const std::size_t& step_pin,
        const std::size_t& dir_pin,
        const std::size_t& m0_pin,
        const std::size_t& m1_pin,
        const std::size_t& m2_pin,
        const Mode& mode,
        const std::size_t& hold_time_ms
    ): m_enable_pin(enable_pin), m_step_pin(step_pin), m_dir_pin(dir_pin),
        m_m0_pin(m0_pin), m_m1_pin(m1_pin), m_m2_pin(m2_pin),
        m_hold_time_ms(hold_time_ms) {
        init_output(m_enable_pin, false);
        init_output(m_step_pin, false);
        init_output(m_dir_pin, false);
        init_output(m_m0_pin, false);
        init_output(m_m1_pin, false);
        init_output(m_m2_pin, false);
        set_mode(mode, m_m0_pin, m_m1_pin, m_m2_pin);
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
        sleep_ms(m_hold_time_ms);
        gpio_put(m_step_pin, false);
    }

    inline void PicoStepper::init_output(const std::size_t& pin_num, const bool init_value) {
        gpio_init(pin_num);
        gpio_set_dir(pin_num, GPIO_OUT);
        gpio_put(pin_num, init_value);
    }

    inline void PicoStepper::set_mode(const Mode& mode, const std::size_t& m0_pin, const std::size_t& m1_pin, const std::size_t& m2_pin) {
        switch (mode) {
        case Mode::FULL_STEP:
            gpio_put(m0_pin, false);
            gpio_put(m1_pin, false);
            gpio_put(m2_pin, false);
            break;
        case Mode::HALF_STEP:
            gpio_put(m0_pin, true);
            gpio_put(m1_pin, false);
            gpio_put(m2_pin, false);
            break;
        case Mode::QUARTER_STEP:
            gpio_put(m0_pin, false);
            gpio_put(m1_pin, true);
            gpio_put(m2_pin, false);
            break;
        default:
            throw std::invalid_argument("unsupported mode received");        
        }
    }
}

#endif // PICO_STEPPER_MOTOR_HPP