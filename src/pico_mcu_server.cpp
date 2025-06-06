#include <string>

#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/regs/intctrl.h"
#include "hardware/uart.h"
#include "pico/stdio.h"

#include "ipc_data.hpp"
#include "manager_instance.hpp"
#include "movement_host_builder.hpp"
#include "pico_axis_controller.hpp"
#include "pico_axis_controller_config.hpp"
#include "raw_data_package_descriptor.hpp"
#include "raw_data_package_reader.hpp"
#include "raw_data_package_utils.hpp"
#include "raw_data_package_writer.hpp"
#include "movement_proto_api_request_parser.hpp"
#include "movement_proto_api_response_serializer.hpp"

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
    const auto response_serializer = MovementProtoApiResponseSerializer();

    auto host_builder = MovementHostBuilder<PicoAxesControllerConfig, RawData>();
	host_builder
        .set_api_request_parser(request_parser)
        .set_raw_data_reader(raw_data_reader_instance)
        .set_api_response_serializer(response_serializer)
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