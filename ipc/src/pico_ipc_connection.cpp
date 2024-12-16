#include "hardware/gpio.h"
#include "hardware/regs/intctrl.h"
#include "hardware/uart.h"

#include "pico_ipc_connection.hpp"

using namespace pico_mcu_ipc;

PicoIpcConnection::BufferedConnection *pico_mcu_ipc::PicoIpcConnection::s_buffered_connection(nullptr);

void PicoIpcConnection::init_uart(const Baud& baud, BufferedConnection *buffered_connection) {
    if (nullptr != s_buffered_connection) {
        throw std::runtime_error("buffered connection is already initialized by another instance");
    }
    uart_init(uart0, baud_to_uint(baud));
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
    uart_set_baudrate(uart0, baud_to_uint(baud));
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, DATA_BITS, STOP_BITS, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart0, false);
    irq_set_exclusive_handler(UART0_IRQ, &on_received_cb);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(uart0, true, false);
    s_buffered_connection = buffered_connection;
}

void PicoIpcConnection::uninit_uart() {
    irq_set_enabled(UART0_IRQ, false);
    uart_set_irq_enables(uart0, false, false);
    uart_deinit(uart0);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_NULL);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_NULL);
    s_buffered_connection = nullptr;
}

void PicoIpcConnection::on_received_cb() {
    RawData data;
    while (uart_is_readable(uart0)) {
        data.push_back(uart_getc(uart0));
    }
    if (!s_buffered_connection) {
        return;
    }
    s_buffered_connection->feed(data);
    irq_clear(UART0_IRQ);
}

void PicoIpcConnection::send_data(const RawData& data) {
    for (auto ch: data) {
        uart_putc(uart0, ch);
    }
}

uint PicoIpcConnection::baud_to_uint(const Baud& baud) {
    switch (baud) {
    case Baud::B9600:
        return 9600;
    case Baud::B115200:
        return 115200;
    default:
        throw std::invalid_argument("unsupported baud");
    }
}