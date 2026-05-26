#ifndef	PICO_AXIS_CONTROLLER_HPP
#define	PICO_AXIS_CONTROLLER_HPP

#include <map>

#include "pico/time.h"

#include "axes_controller.hpp"
#include "manager_instance.hpp"
#include "movement_manager_data.hpp"
#include "pico_axis_controller_config.hpp"
#include "pico_stepper_motor.hpp"

namespace pico {
    class PicoAxisController: public manager::AxesController {
    public:
        PicoAxisController(const PicoAxesControllerConfig& config);
        PicoAxisController(const PicoAxisController&) = delete;
        PicoAxisController& operator=(const PicoAxisController&) = delete;
        ~PicoAxisController() noexcept override;
        
        void step(const manager::Axis& axis, const manager::Direction& direction, const double duration) override;
        void enable() override;
        void disable() override;
        double get_step_length(const manager::Axis& axis) const override;
    private:
        PicoAxesControllerConfig m_config;
        using Steppers = std::map<manager::Axis, manager::Instance<PicoStepper>>;
        Steppers m_steppers;

        static void disable_steppers(Steppers *steppers);
        static void enable_steppers(Steppers *steppers);
        static Steppers create_steppers(const PicoAxesControllerConfig& config);
    };

    inline PicoAxisController::PicoAxisController(const PicoAxesControllerConfig& config): m_config(config), m_steppers(create_steppers(config)) {
        disable_steppers(&m_steppers);
    }

    inline PicoAxisController::~PicoAxisController() noexcept {
        disable_steppers(&m_steppers);
    }

    inline void PicoAxisController::step(const manager::Axis& axis, const manager::Direction& direction, const double duration) {
        auto& descriptor = m_steppers.at(axis);
        const auto rotational_dir = m_config.axis_config(axis).directions_mapping.at(direction);
        const auto duration_ms = static_cast<uint32_t>(1000.0 * duration);

        m_steppers.at(axis).get().step(rotational_dir);
        sleep_ms(duration_ms);
    }

    inline void PicoAxisController::enable() {
        enable_steppers(&m_steppers);
    }

    inline void PicoAxisController::disable() {
        disable_steppers(&m_steppers);
    }

    inline double PicoAxisController::get_step_length(const manager::Axis& axis) const {
        return m_config.axis_config(axis).step_length;
    }

    inline void PicoAxisController::disable_steppers(Steppers *steppers) {
        for (auto& [axis, stepper_instance]: *steppers) {
            stepper_instance.get().set_state(PicoStepper::State::DISABLED);
        }
    }

    inline void PicoAxisController::enable_steppers(Steppers *steppers) {
        for (auto& [axis, stepper_instance]: *steppers) {
            stepper_instance.get().set_state(PicoStepper::State::ENABLED);
        }
    }

    inline typename PicoAxisController::Steppers PicoAxisController::create_steppers(const PicoAxesControllerConfig& config) {
        using namespace manager;
        Steppers steppers;
        for (const auto& axis: {Axis::X, Axis::Y, Axis::Z}) {
            const auto stepper_config = config.axis_config(axis).stepper_config;
            steppers.insert(
                {
                    axis,
                    Instance<PicoStepper>(new PicoStepper(stepper_config))
                }
            );
        }
        return steppers;
    }
}

#endif // PICO_AXIS_CONTROLLER_HPP