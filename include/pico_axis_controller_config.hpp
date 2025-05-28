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
        virtual ~PicoAxesControllerConfig() noexcept = default;
        
        AxisConfig axis_config(const manager::Axis& axis) const;
    private:
        AxesConfig m_axes_config;
    };

    inline PicoAxesControllerConfig::PicoAxesControllerConfig(const AxesConfig& axes_config): m_axes_config(axes_config) {

    }

    inline AxisConfig PicoAxesControllerConfig::axis_config(const manager::Axis& axis) const {
        auto it = m_axes_config.find(axis);
        if (it == m_axes_config.end()) {
            throw std::invalid_argument("axis config not found for axis tag " + std::to_string(static_cast<int>(axis)));
        }
        return it->second;
    }
}

#endif // PICO_AXIS_CONTROLLER_CONFIG_HPP