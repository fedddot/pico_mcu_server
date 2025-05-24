#ifndef AXIS_CONFIG_HPP
#define AXIS_CONFIG_HPP

#include <map>

#include "movement_manager_data.hpp"
#include "pico_stepper_motor.hpp"

namespace pico {
    struct AxisConfig {
        using DirectionsMapping = std::map<manager::Direction, PicoStepper::Direction>;
        AxisConfig(
            const double step_length = 0.1,
            const DirectionsMapping& directions_mapping = DirectionsMapping {
                {manager::Direction::NEGATIVE, PicoStepper::Direction::CCW},
                {manager::Direction::POSITIVE, PicoStepper::Direction::CW}
            }
        );
        AxisConfig(const AxisConfig&) = default;
        AxisConfig& operator=(const AxisConfig&) = default;
        virtual ~AxisConfig() noexcept = default;
        
        double step_length;
        DirectionsMapping directions_mapping;
    };

    inline AxisConfig::AxisConfig(
        const double step_length,
        const DirectionsMapping& directions_mapping
    ): step_length(step_length), directions_mapping(directions_mapping) {
        if (step_length <= 0.0) {
            throw std::invalid_argument("step_length must be positive");
        }
        for (const auto& direction: {manager::Direction::NEGATIVE, manager::Direction::POSITIVE}) {
            if (directions_mapping.find(direction) == directions_mapping.end()) {
                throw std::invalid_argument("correspondence is not established for one of directions");
            }
        }
    }
} // namespace pico

#endif // AXIS_CONFIG_HPP