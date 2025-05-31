#include <string>

#include "json/reader.h"
#include "json/value.h"
#include "json/writer.h"

#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/regs/intctrl.h"
#include "hardware/uart.h"
#include "pico/stdio.h"

#include "axis_config.hpp"
#include "ipc_data.hpp"
#include "ipc_instance.hpp"
#include "manager_instance.hpp"
#include "movement_host_builder.hpp"
#include "movement_json_api_request_parser.hpp"
#include "movement_json_api_response_serializer.hpp"
#include "movement_vendor_api_request.hpp"
#include "movement_vendor_api_response.hpp"
#include "pico_axis_controller.hpp"
#include "pico_axis_controller_config.hpp"
#include "pico_stepper_motor.hpp"
#include "raw_data_package_descriptor.hpp"
#include "raw_data_package_reader.hpp"
#include "raw_data_package_utils.hpp"
#include "raw_data_package_writer.hpp"
#include "movement_proto_api_request_parser.hpp"

#ifndef MSG_PREAMBLE
#   error "MSG_PREAMBLE is not defined"
#endif

#ifndef MSG_SIZE_FIELD_LEN
#   error "MSG_SIZE_FIELD_LEN is not defined"
#endif

#ifndef PICO_IPC_BAUD
#   error "PICO_IPC_BAUD is not defined"
#endif

#define BUFFER_SIZE_INCREMENT 100UL

using namespace ipc;
using namespace host;
using namespace manager;
using namespace pico;

static auto s_raw_data_buffer = RawData();

static manager::Instance<AxesController> create_axes_controller(const PicoAxesControllerConfig& config);
static RawData serialize_api_response(const vendor::MovementVendorApiResponse& response);


static void write_raw_data(const RawData& data);
static void init_uart_listener();

int main(void) {
    s_raw_data_buffer.reserve(BUFFER_SIZE_INCREMENT);

    const auto preamble_str = std::string(MSG_PREAMBLE);
    const auto package_descriptor = RawDataPackageDescriptor(
        RawData(preamble_str.begin(), preamble_str.end()),
        MSG_SIZE_FIELD_LEN
    );
    const auto raw_data_reader_instance = MovementHostBuilder<PicoAxesControllerConfig, RawData>::RawDataReaderInstance(
        new RawDataPackageReader(
            &s_raw_data_buffer,
            package_descriptor,
            parse_package_size
        )
    );
    const auto raw_data_writer_instance = MovementHostBuilder<PicoAxesControllerConfig, RawData>::RawDataWriterInstance(
        new RawDataPackageWriter(
            package_descriptor,
            serialize_package_size,
            write_raw_data
        )
    );

    const auto request_parser = MovementProtoApiRequestParser();

    auto host_builder = MovementHostBuilder<PicoAxesControllerConfig, RawData>();
	host_builder
        .set_api_request_parser(request_parser)
        .set_raw_data_reader(raw_data_reader_instance)
        .set_api_response_serializer(serialize_api_response)
        .set_raw_data_writer(raw_data_writer_instance)
        .set_axes_controller_creator(create_axes_controller);
	
        
    stdio_init_all();
    init_uart_listener();
        
    auto host = host_builder.build();
    while (true) {
        host.run_once();
    }
    return 0;
}

inline manager::Instance<AxesController> create_axes_controller(const PicoAxesControllerConfig& config) {
    return manager::Instance<AxesController>(
        new PicoAxisController(config)
    );
}

inline Json::Value retrieve_val_from_object(const Json::Value& json_val, const std::string& key) {
    if (!json_val.isMember(key)) {
        throw std::invalid_argument("missing " + key + " key in json data");
    }
    return json_val[key];
}

inline typename PicoStepper::Config parse_stepper_cfg(const Json::Value& json_data) {
    return PicoStepper::Config {
        .enable_pin = retrieve_val_from_object(json_data, "enable_pin").asUInt(),
        .step_pin = retrieve_val_from_object(json_data, "step_pin").asUInt(),
        .dir_pin = retrieve_val_from_object(json_data, "dir_pin").asUInt(),
        .hold_time_us = retrieve_val_from_object(json_data, "hold_time_us").asUInt(),
    };
}

inline typename AxisConfig::DirectionsMapping parse_directions_mapping(const Json::Value& json_data) {
    const auto linear_directions = std::map<std::string, manager::Direction> {
        {"NEGATIVE", manager::Direction::NEGATIVE},
        {"POSITIVE", manager::Direction::POSITIVE},
    };
    const auto rotational_directions = std::map<std::string, PicoStepper::Direction> {
        {"CCW", PicoStepper::Direction::CCW},
        {"CW", PicoStepper::Direction::CW},
    };
    AxisConfig::DirectionsMapping directions_mapping;
    for (const auto& [linear_direction_str, linear_direction]: linear_directions) {
        if (!json_data.isMember(linear_direction_str)) {
            throw std::invalid_argument("missing " + linear_direction_str + " key in json data");
        }
        const auto rotational_direction_str = json_data[linear_direction_str].asString();
        if (rotational_directions.find(rotational_direction_str) == rotational_directions.end()) {
            throw std::invalid_argument("invalid rotational direction: " + rotational_direction_str);
        }
        directions_mapping[linear_direction] = rotational_directions.at(rotational_direction_str);
    }
    return directions_mapping;
}

inline AxisConfig parse_axis_cfg(const Json::Value& json_data) {
    const auto stepper_cfg_json = retrieve_val_from_object(json_data, "stepper_cfg");
    const auto stepper_cfg = parse_stepper_cfg(stepper_cfg_json);

    const auto step_length = retrieve_val_from_object(json_data, "step_length").asDouble();
    const auto directions_mapping_json = retrieve_val_from_object(json_data, "directions_mapping");
    const auto directions_mapping = parse_directions_mapping(directions_mapping_json);
    return AxisConfig(stepper_cfg, step_length, directions_mapping);
}

inline PicoAxesControllerConfig parse_axes_controller_cfg(const Json::Value& json_data) {
    const auto axes_mapping = std::map<std::string, manager::Axis> {
        {"x", manager::Axis::X},
        {"y", manager::Axis::Y},
        {"z", manager::Axis::Z},
    };
    PicoAxesControllerConfig::AxesConfig axes_config;
    for (const auto& [axis_str, axis]: axes_mapping) {
        const auto axis_cfg_json = retrieve_val_from_object(json_data, axis_str);
        const auto axis_cfg = parse_axis_cfg(axis_cfg_json);
        axes_config.insert({ axis, axis_cfg });
    }
    return PicoAxesControllerConfig(axes_config);
}

inline ipc::Instance<vendor::MovementVendorApiRequest> parse_api_request(const RawData& raw_data) {
    const auto json_parser = MovementJsonApiRequestParser<PicoAxesControllerConfig>(parse_axes_controller_cfg);
    Json::Value json_val;
    Json::Reader reader;
    if (!reader.parse(std::string(raw_data.begin(), raw_data.end()), std::ref(json_val), true)) {
        throw std::runtime_error("failed to parse raw ipc_data data: " + reader.getFormattedErrorMessages());
    }
    return json_parser(json_val);
}

inline RawData serialize_api_response(const vendor::MovementVendorApiResponse& response) {
    const auto json_serializer = MovementJsonApiResponseSerializer();
    const auto json_response = json_serializer(response);
    const auto writer_builder = Json::StreamWriterBuilder();
    const auto serial_str = Json::writeString(writer_builder, json_response);
    return RawData(serial_str.begin(), serial_str.end());
}

inline void write_raw_data(const RawData& data) {
    for (const auto ch: data) {
        uart_putc(uart0, ch);
    }
}

inline void on_received_cb() {
    while (uart_is_readable(uart0)) {
        if (s_raw_data_buffer.size() >= s_raw_data_buffer.capacity()) {
            s_raw_data_buffer.reserve(s_raw_data_buffer.capacity() + BUFFER_SIZE_INCREMENT);
        }
        s_raw_data_buffer.push_back(uart_getc(uart0));
    }
    irq_clear(UART0_IRQ);
}

inline void init_uart_listener() {
    enum : uint {
        UART0_TX_PIN = 0,
        UART0_RX_PIN = 1,
        DATA_BITS = 8,
        STOP_BITS = 1
    };
    uart_init(uart0, PICO_IPC_BAUD);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
    uart_set_baudrate(uart0, PICO_IPC_BAUD);
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, DATA_BITS, STOP_BITS, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart0, false);
    irq_set_exclusive_handler(UART0_IRQ, &on_received_cb);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(uart0, true, false);
}