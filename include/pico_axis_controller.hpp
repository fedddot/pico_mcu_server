#ifndef	PICO_AXIS_CONTROLLER_HPP
#define	PICO_AXIS_CONTROLLER_HPP

#include <map>
#include <stdexcept>

#include "axes_controller.hpp"
#include "manager_instance.hpp"
#include "pico/time.h"

#include "movement_manager_data.hpp"

#include "pico_stepper_motor.hpp"

namespace pico {
    class PicoAxisController: public manager::AxesController {
    public:
        struct StepperMotorDescriptor {
            manager::Instance<PicoStepper> stepper;
            std::map<manager::Direction, PicoStepper::Direction> directions;
        };
        using Steppers = std::map<manager::Axis, StepperMotorDescriptor>;
        PicoAxisController(
            const Steppers& steppers
        );
        PicoAxisController(const PicoAxisController&) = default;
        PicoAxisController& operator=(const PicoAxisController&) = default;
        
        ~PicoAxisController() noexcept override;
        void step(const manager::Axis& axis, const manager::Direction& direction, const double duration) override;
        void enable() override;
        void disable() override;
        virtual double get_step_length(const manager::Axis& axis) const override;
    private:
        manager::AxesProperties m_axes_properties;
        Steppers m_steppers;

        static void disable_steppers(Steppers *steppers);
        static void enable_steppers(Steppers *steppers);
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
            for (const auto& direction: {Direction::NEGATIVE, Direction::POSITIVE}) {
                const auto dir_iter = (iter->second).directions.find(direction);
                if ((iter->second).directions.end() == dir_iter) {
                    throw std::invalid_argument("correspondence is not established for one of directions");
                }
            }            
        }
        disable_steppers(&m_steppers);
    }

    inline PicoAxisController::~PicoAxisController() noexcept {
        disable_steppers(&m_steppers);
    }

    inline void PicoAxisController::step(const manager::AxisStep& step) {
        auto& descriptor = m_steppers.at(step.axis);
        const auto rotational_dir = descriptor.directions.at(step.direction);
        const auto duration_ms = static_cast<uint32_t>(1000.0 * step.duration);

        descriptor.stepper.get().step(rotational_dir);
        sleep_ms(duration_ms);
    }

    inline void PicoAxisController::enable() {
        enable_steppers(&m_steppers);
    }

    inline void PicoAxisController::disable() {
        disable_steppers(&m_steppers);
    }

    inline manager::AxesController *PicoAxisController::clone() const {
        return new PicoAxisController(*this);
    }

    inline void PicoAxisController::disable_steppers(Steppers *steppers) {
        for (auto& [axis, stepper_dsc]: *steppers) {
            stepper_dsc.stepper.get().set_state(manager::State::DISABLED);
        }
    }
    inline void PicoAxisController::enable_steppers(Steppers *steppers) {
        for (auto& [axis, stepper_dsc]: *steppers) {
            stepper_dsc.stepper.get().set_state(manager::State::ENABLED);
        }
    }
}

#endif // PICO_AXIS_CONTROLLER_HPP