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
        struct StepperMotorDescriptor {
            std::shared_ptr<manager::StepperMotor> stepper_ptr;
            std::map<manager::Direction, manager::RotationDirection> directions;
        };
        using Steppers = std::map<manager::Axis, StepperMotorDescriptor>;
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
        using namespace manager;
        for (const auto& axis: {Axis::X, Axis::Y, Axis::Z}) {
            const auto iter = m_steppers.find(axis);
            if (m_steppers.end() == iter) {
                throw std::invalid_argument("stepper is not assigned for one of axes");
            }
            if (!(iter->second).stepper_ptr) {
                throw std::invalid_argument("invalid stepper ptr received");
            }
            for (const auto& direction: {Direction::NEGATIVE, Direction::POSITIVE}) {
                const auto dir_iter = (iter->second).directions.find(direction);
                if ((iter->second).directions.end() == dir_iter) {
                    throw std::invalid_argument("correspondence is not established for one of directions");
                }
            }            
        }
        disable_steppers(m_steppers);
    }

    inline PicoAxisController::~PicoAxisController() noexcept {
        disable_steppers(m_steppers);
    }

    inline void PicoAxisController::step(const manager::AxisStep& step) {
        throw std::runtime_error("PicoAxisController::step not implemented");
    }

    inline void PicoAxisController::enable() {
        enable_steppers(m_steppers);
    }

    inline void PicoAxisController::disable() {
        disable_steppers(m_steppers);
    }

    inline void PicoAxisController::disable_steppers(const Steppers& steppers) {
        for (const auto& [axis, stepper_dsc]: steppers) {
            stepper_dsc.stepper_ptr->set_state(manager::State::DISABLED);
        }
    }
    inline void PicoAxisController::enable_steppers(const Steppers& steppers) {
        for (const auto& [axis, stepper_dsc]: steppers) {
            stepper_dsc.stepper_ptr->set_state(manager::State::ENABLED);
        }
    }
}

#endif // PICO_AXIS_CONTROLLER_HPP