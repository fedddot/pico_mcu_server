#ifndef MOVEMENT_PROTO_API_REQUEST_PARSER_HPP
#define MOVEMENT_PROTO_API_REQUEST_PARSER_HPP

#include <stdexcept>

#include "pb_decode.h"

#include "axes_controller_config_request.hpp"
#include "axis_config.hpp"
#include "ipc_data.hpp"
#include "ipc_instance.hpp"
#include "linear_movement_request.hpp"
#include "movement_manager_data.hpp"
#include "movement_vendor_api.pb.h"
#include "movement_vendor_api_request.hpp"
#include "pico_axis_controller_config.hpp"
#include "rotation_movement_request.hpp"

namespace ipc {
    class MovementProtoApiRequestParser {
    public:
        MovementProtoApiRequestParser() = default;
        MovementProtoApiRequestParser(const MovementProtoApiRequestParser&) = default;
        MovementProtoApiRequestParser& operator=(const MovementProtoApiRequestParser&) = default;
        virtual ~MovementProtoApiRequestParser() noexcept = default;

        Instance<vendor::MovementVendorApiRequest> operator()(const RawData& raw_data) const;
    private:
        static Instance<vendor::MovementVendorApiRequest> parse_config_request(const movement_vendor_api_MovementApiRequest& pb_request);
        static Instance<vendor::MovementVendorApiRequest> parse_linear_movement_request(const movement_vendor_api_MovementApiRequest& pb_request);
        static Instance<vendor::MovementVendorApiRequest> parse_rotational_movement_request(const movement_vendor_api_MovementApiRequest& pb_request);
        static pico::AxisConfig parse_axis_config(const movement_vendor_api_AxisConfig& pb_axis_config);
        static pico::AxisConfig::DirectionsMapping parse_directions_mapping(const movement_vendor_api_DirectionsMapping& pb_directions);
    };

    inline Instance<vendor::MovementVendorApiRequest> MovementProtoApiRequestParser::operator()(const RawData& raw_data) const {
        auto istream = pb_istream_from_buffer(
            (const pb_byte_t *)raw_data.data(),
            raw_data.size()
        );
        movement_vendor_api_MovementApiRequest decoded_request = movement_vendor_api_MovementApiRequest_init_default;
	    if (!pb_decode(&istream, movement_vendor_api_MovementApiRequest_fields, &decoded_request)) {
            throw std::runtime_error("Failed to decode MovementApiRequest from raw data: " + std::string(PB_GET_ERROR(&istream)));
        }
        switch (decoded_request.which_request) {
        case movement_vendor_api_MovementApiRequest_config_request_tag:
            return parse_config_request(decoded_request);
        case movement_vendor_api_MovementApiRequest_linear_movement_request_tag:
            return parse_linear_movement_request(decoded_request);
        default:
            throw std::runtime_error("unsupported request type in MovementApiRequest");
        }
    }

    inline Instance<vendor::MovementVendorApiRequest> MovementProtoApiRequestParser::parse_config_request(const movement_vendor_api_MovementApiRequest& pb_request) {
        const auto& pb_cfg_request = pb_request.request.config_request;
        if (!pb_cfg_request.has_axes_config) {
            throw std::invalid_argument("AxesControllerConfigApiRequest is missing axes configuration");
        }
        const auto& pb_axes_config = pb_cfg_request.axes_config;
        const auto axes_config = pico::PicoAxesControllerConfig::AxesConfig {
            {manager::Axis::X, parse_axis_config(pb_axes_config.x_axis_cfg)},
            {manager::Axis::Y, parse_axis_config(pb_axes_config.y_axis_cfg)},
            {manager::Axis::Z, parse_axis_config(pb_axes_config.z_axis_cfg)},
        };
        return Instance<vendor::MovementVendorApiRequest>(new vendor::AxesControllerConfigApiRequest<pico::PicoAxesControllerConfig>(axes_config));
    }

    inline Instance<vendor::MovementVendorApiRequest> MovementProtoApiRequestParser::parse_linear_movement_request(const movement_vendor_api_MovementApiRequest& pb_request) {
        const auto& pb_linear_request = pb_request.request.linear_movement_request;
        if (!pb_linear_request.has_target) {
            throw std::invalid_argument("LinearMovementRequest is missing target vector");
        }
        const auto target = manager::Vector<double> (
            static_cast<double>(pb_linear_request.target.x),
            static_cast<double>(pb_linear_request.target.y),
            static_cast<double>(pb_linear_request.target.z)
        );
        const auto speed = static_cast<double>(pb_linear_request.speed);
        return Instance<vendor::MovementVendorApiRequest>(new vendor::LinearMovementRequest(target, speed));
    }

    inline Instance<vendor::MovementVendorApiRequest> MovementProtoApiRequestParser::parse_rotational_movement_request(const movement_vendor_api_MovementApiRequest& pb_request) {
        const auto& pb_rotation_request = pb_request.request.rotation_movement_request;
        if (!pb_rotation_request.has_target) {
            throw std::invalid_argument("RotationMovementRequest is missing target vector");
        }
        const auto target = manager::Vector<double> (
            static_cast<double>(pb_rotation_request.target.x),
            static_cast<double>(pb_rotation_request.target.y),
            static_cast<double>(pb_rotation_request.target.z)
        );
        if (!pb_rotation_request.has_rotation_center) {
            throw std::invalid_argument("RotationMovementRequest is missing rotation center vector");
        }
        const auto rotation_center = manager::Vector<double> (
            static_cast<double>(pb_rotation_request.rotation_center.x),
            static_cast<double>(pb_rotation_request.rotation_center.y),
            static_cast<double>(pb_rotation_request.rotation_center.z)
        );
        const auto speed = static_cast<double>(pb_rotation_request.speed);
        const auto angle = static_cast<double>(pb_rotation_request.angle);
        return Instance<vendor::MovementVendorApiRequest>(new vendor::RotationMovementRequest(target, rotation_center, angle, speed));
    }

    inline pico::AxisConfig MovementProtoApiRequestParser::parse_axis_config(const movement_vendor_api_AxisConfig& pb_axis_config) {
        if (!pb_axis_config.has_stepper_config) {
            throw std::invalid_argument("AxisConfig is missing stepper configuration");
        }
        if (!pb_axis_config.has_directions_mapping) {
            throw std::invalid_argument("AxisConfig is missing directions mapping");
        }
        return pico::AxisConfig(
            pico::PicoStepper::Config {
                .enable_pin = pb_axis_config.stepper_config.enable_pin,
                .step_pin = pb_axis_config.stepper_config.step_pin,
                .dir_pin = pb_axis_config.stepper_config.dir_pin,
                .hold_time_us = pb_axis_config.stepper_config.hold_time_us
            },
            static_cast<double>(pb_axis_config.step_length),
            parse_directions_mapping(pb_axis_config.directions_mapping)
        );
    }

    inline pico::AxisConfig::DirectionsMapping MovementProtoApiRequestParser::parse_directions_mapping(const movement_vendor_api_DirectionsMapping& pb_directions) {
        const auto directions_mapping = std::map<movement_vendor_api_StepperDirection, pico::PicoStepper::Direction> {
            {movement_vendor_api_StepperDirection_CW, pico::PicoStepper::Direction::CW},
            {movement_vendor_api_StepperDirection_CCW, pico::PicoStepper::Direction::CCW},
        };
        return pico::AxisConfig::DirectionsMapping {
            { manager::Direction::NEGATIVE, directions_mapping.at(pb_directions.negative) },
            { manager::Direction::POSITIVE, directions_mapping.at(pb_directions.positive) },
        };
    }
}

#endif // MOVEMENT_PROTO_API_REQUEST_PARSER_HPP