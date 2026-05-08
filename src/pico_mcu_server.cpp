#include "json/value.h"
#include <cstdint>
#include <stdexcept>

#include "driver_mpu6050.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/regs/intctrl.h"
#include "hardware/uart.h"
#include "pico/stdio.h"

#include "ring_buffer.hpp"
#include "cobs_frame_reader.hpp"
#include "cobs_frame_writer.hpp"
#include "json_message_reader.hpp"
#include "json_message_writer.hpp"
#include "driver_mpu6050_pico.hpp"

#include "sd_io.h"

#ifndef BUFF_SIZE
#   error "BUFF_SIZE is not defined"
#endif 

#ifndef PICO_IPC_BAUD
#   error "PICO_IPC_BAUD is not defined"
#endif

using namespace nanoipc;
using namespace pico;

static RingBuffer<BUFF_SIZE> s_raw_data_buffer;

static void write_raw_data(const std::uint8_t *data, const std::size_t size);
static void init_uart_listener();

int main(void) {
    CobsFrameReader cobs_frame_reader(&s_raw_data_buffer);
    JsonMessageReader json_message_reader(&cobs_frame_reader);

    const CobsFrameWriter cobs_frame_writer(&write_raw_data);
    JsonMessageWriter json_message_writer(&cobs_frame_writer);

    stdio_init_all();
    init_uart_listener();

    SD_DEV sd_card;
    const auto sd_init_res = SD_Init(&sd_card);
    if (sd_init_res != SDRESULTS::SD_OK) {
        throw std::runtime_error("Failed to initialize SD card");
    }

    // MPU6050_ADDRESS_AD0_LOW = 0xD0, MPU6050_ADDRESS_AD0_HIGH = 0xD2
    DriverMpu6050Pico gyro(mpu6050_address_t::MPU6050_ADDRESS_AD0_LOW);
    
    while (true) {
        const auto msg = json_message_reader.read();
        if (!msg.has_value()) {
            continue;
        }
        auto resp = Json::Value(Json::objectValue);
        resp["temperature"] = gyro.read_temp();
        resp["accel_x"] = gyro.read_accel(mpu6050_axis_t::MPU6050_AXIS_X);
        resp["accel_y"] = gyro.read_accel(mpu6050_axis_t::MPU6050_AXIS_Y);
        resp["accel_z"] = gyro.read_accel(mpu6050_axis_t::MPU6050_AXIS_Z);
        resp["gyro_x"] = gyro.read_gyro(mpu6050_axis_t::MPU6050_AXIS_X);
        resp["gyro_y"] = gyro.read_gyro(mpu6050_axis_t::MPU6050_AXIS_Y);
        resp["gyro_z"] = gyro.read_gyro(mpu6050_axis_t::MPU6050_AXIS_Z);
        json_message_writer.write(resp);
    }
    return 0;
}

inline void write_raw_data(const std::uint8_t *data, const std::size_t size) {
    for (std::size_t i = 0; i < size; ++i) {
        uart_putc(uart0, data[i]);
    }
}

inline void on_received_cb() {
    while (uart_is_readable(uart0)) {
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