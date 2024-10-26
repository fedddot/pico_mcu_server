#include "hardware/gpio.h"
#include "hardware/regs/intctrl.h"
#include "hardware/uart.h"

#include "pico_synchronous_ipc_connection.hpp"

using namespace pico_mcu_ipc;

void PicoSynchronousIpcConnection::init_uart(const Baud& baud, BufferedConnection *buffered_connection) {
    uart_init(uart0, baud_to_uint(baud));
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
    uart_set_baudrate(uart0, baud_to_uint(baud));
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, DATA_BITS, STOP_BITS, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart0, false);
    irq_set_enabled(UART0_IRQ, false);
    uart_set_irq_enables(uart0, false, false);
}

void PicoSynchronousIpcConnection::uninit_uart() {
    irq_set_enabled(UART0_IRQ, false);
    uart_set_irq_enables(uart0, false, false);
    uart_deinit(uart0);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_NULL);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_NULL);
}

void PicoSynchronousIpcConnection::loop() {
    RawData data;
    while (uart_is_readable(uart0)) {
        data.push_back(uart_getc(uart0));
    }
    m_buffered_connection.feed(data);
}

void PicoSynchronousIpcConnection::send_data(const RawData& data) {
    for (auto ch: data) {
        uart_putc(uart0, ch);
    }
}

uint PicoSynchronousIpcConnection::baud_to_uint(const Baud& baud) {
    switch (baud) {
    case Baud::B9600:
        return 9600;
    case Baud::B115200:
        return 115200;
    default:
        throw std::invalid_argument("unsupported baud");
    }
}