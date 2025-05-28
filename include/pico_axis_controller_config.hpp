#ifndef	PICO_AXIS_CONTROLLER_CONFIG_HPP
#define	PICO_AXIS_CONTROLLER_CONFIG_HPP

#include <map>
#include <stdexcept>

#include "axis_config.hpp"
#include "movement_manager_data.hpp"

namespace pico {
    class PicoAxesControllerConfig {
    public:
        using AxesConfig = std::map<manager::Axis, AxisConfig>;
        PicoAxesControllerConfig(const AxesConfig& axes_config);
        PicoAxesControllerConfig(const PicoAxesControllerConfig&) = default;
        PicoAxesControllerConfig& operator=(const PicoAxesControllerConfig&) = default;
        
        ~PicoAxesControllerConfig() noexcept = default;

        
    private:
        manager::AxesProperties m_axes_properties;
        Steppers m_steppers;

        static void disable_steppers(Steppers *steppers);
        static void enable_steppers(Steppers *steppers);
    };

    inline PicoAxesControllerConfig::PicoAxesControllerConfig(
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

    inline PicoAxesControllerConfig::~PicoAxesControllerConfig() noexcept {
        disable_steppers(&m_steppers);
    }

    inline void PicoAxesControllerConfig::step(const manager::AxisStep& step) {
        auto& descriptor = m_steppers.at(step.axis);
        const auto rotational_dir = descriptor.directions.at(step.direction);
        const auto duration_ms = static_cast<uint32_t>(1000.0 * step.duration);

        descriptor.stepper.get().step(rotational_dir);
        sleep_ms(duration_ms);
    }

    inline void PicoAxesControllerConfig::enable() {
        enable_steppers(&m_steppers);
    }

    inline void PicoAxesControllerConfig::disable() {
        disable_steppers(&m_steppers);
    }

    inline manager::AxesController *PicoAxesControllerConfig::clone() const {
        return new PicoAxesControllerConfig(*this);
    }

    inline void PicoAxesControllerConfig::disable_steppers(Steppers *steppers) {
        for (auto& [axis, stepper_dsc]: *steppers) {
            stepper_dsc.stepper.get().set_state(manager::State::DISABLED);
        }
    }
    inline void PicoAxesControllerConfig::enable_steppers(Steppers *steppers) {
        for (auto& [axis, stepper_dsc]: *steppers) {
            stepper_dsc.stepper.get().set_state(manager::State::ENABLED);
        }
    }
}

#endif // PICO_AXIS_CONTROLLER_CONFIG_HPP