#ifndef	PICO_AXIS_CONTROLLER_HPP
#define	PICO_AXIS_CONTROLLER_HPP

#include <memory>
#include <stdexcept>

#include "movement_manager.hpp"
#include "movement_manager_data.hpp"

#include "stepper_motor.hpp"
#include "stepper_motor_data.hpp"

namespace pico {
    class PicoAxisController: public manager::MovementManager::AxesController {
    public:
        using Steppers = std::map<manager::Axis, std::shared_ptr<manager::StepperMotor>>;
        PicoAxisController(
            const manager::AxesProperties& axes_properties,
            const Steppers& steppers
        );
        PicoAxisController(const PicoAxisController&) = default;
        PicoAxisController& operator=(const PicoAxisController&) = default;
        
        ~PicoAxisController() noexcept override;
        void step(const manager::AxisStep& step) override;
        void enable() override;
        void disable() override;
        AxesController *clone() const override;
    private:
        manager::AxesProperties m_axes_properties;
        Steppers m_steppers;

        static void disable_steppers(const Steppers& steppers);
        static void enable_steppers(const Steppers& steppers);
    };

    inline PicoAxisController::PicoAxisController(
        const manager::AxesProperties& axes_properties,
        const Steppers& steppers
    ): m_axes_properties(axes_properties), m_steppers(steppers) {
        for (const auto& axis: {manager::Axis::X, manager::Axis::Y, manager::Axis::Z}) {
            const auto iter = m_steppers.find(axis);
            if (m_steppers.end() == iter) {
                throw std::invalid_argument("stepper is not assigned for one of axes");
            }
            auto stepper_ptr = (iter->second).get();
            if (!stepper_ptr) {
                throw std::invalid_argument("invalid stepper ptr received");
            }
        }
        disable_steppers(m_steppers);
    }

    inline PicoAxisController::~PicoAxisController() noexcept {
        disable_steppers(m_steppers);
    }

    inline void PicoAxisController::step(const manager::AxisStep& step) {

    }

    inline void PicoAxisController::enable() {
        enable_steppers(m_steppers);
    }

    inline void PicoAxisController::disable() {
        disable_steppers(m_steppers);
    }

    inline void PicoAxisController::disable_steppers(const Steppers& steppers) {
        for (const auto& [axis, stepper_ptr]: steppers) {
            stepper_ptr->set_state(manager::State::DISABLED);
        }
    }
    inline void PicoAxisController::enable_steppers(const Steppers& steppers) {
        for (const auto& [axis, stepper_ptr]: steppers) {
            stepper_ptr->set_state(manager::State::ENABLED);
        }
    }
}

#endif // PICO_AXIS_CONTROLLER_HPP