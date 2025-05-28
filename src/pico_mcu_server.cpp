#include "json/reader.h"
#include <string>

#include "json/writer.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/regs/intctrl.h"
#include "hardware/uart.h"
#include "pico/stdio.h"

#include "ipc_data.hpp"
#include "ipc_instance.hpp"
#include "manager_instance.hpp"
#include "movement_host_builder.hpp"
#include "movement_vendor_api_request.hpp"
#include "movement_vendor_api_response.hpp"
#include "pico_axis_controller.hpp"
#include "pico_axis_controller_config.hpp"
#include "pico_stepper_motor.hpp"
#include "raw_data_package_descriptor.hpp"
#include "movement_json_api_response_serializer.hpp"
#include "movement_json_api_request_parser.hpp"

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
static ipc::Instance<vendor::MovementVendorApiRequest> parse_api_request(const RawData& raw_data);
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
    auto host_builder = MovementHostBuilder<PicoAxesControllerConfig, RawData>();
	host_builder
        .set_api_request_parser(parse_api_request)
        .set_api_response_serializer(serialize_api_response)
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

inline double retrieve_double(const Json::Value& json_val, const std::string& key) {
    if (!json_val.isMember(key)) {
        throw std::invalid_argument("missing " + key + " key in json data");
    }
    const auto val = json_val[key];
    if (!val.isDouble()) {
        throw std::invalid_argument(key + " key in json data is not a double");
    }
    return val.asDouble();
}

inline int retrieve_int(const Json::Value& json_val, const std::string& key) {
    if (!json_val.isMember(key)) {
        throw std::invalid_argument("missing " + key + " key in json data");
    }
    const auto val = json_val[key];
    if (!val.isInt()) {
        throw std::invalid_argument(key + " key in json data is not an integer");
    }
    return val.asInt();
}

inline typename PicoStepper::Config parse_stepper_cfg(const Json::Value& json_data) {
    return PicoStepper::Config {
        .enable_pin = static_cast<size_t>(retrieve_int(json_data, "enable_pin")),
        .step_pin = static_cast<size_t>(retrieve_int(json_data, "step_pin")),
        .dir_pin = static_cast<size_t>(retrieve_int(json_data, "dir_pin")),
        .hold_time_us = static_cast<size_t>(retrieve_int(json_data, "hold_time_us")),
    };
}

inline ipc::Instance<vendor::MovementVendorApiRequest> parse_api_request(const RawData& raw_data) {
    const auto json_parser = MovementJsonApiRequestParser<PicoAxesControllerConfig>(
        [](const Json::Value& json_data) -> PicoAxesControllerConfig {

            return PicoAxesControllerConfig()
                retrieve_double(json_data, "x_step_length"),
                retrieve_double(json_data, "y_step_length"),
                retrieve_double(json_data, "z_step_length")
            );
        }
    );
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