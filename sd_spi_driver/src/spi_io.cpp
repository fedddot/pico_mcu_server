/*
 * spi_io.c - ulibSD SPI I/O implementation for Raspberry Pi Pico (RP2040)
 *
 * Pin assignments (SPI1):
 *   SCK  -> GP10
 *   MOSI -> GP11
 *   MISO -> GP12
 *   CS   -> GP13 (manual GPIO — SD cards require CS held low across a full command)
 */

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/time.h"
#include <cstddef>
#include <optional>
 
extern "C" {
    #include "spi_io.h"
}

#define SD_SPI_INST     spi1
#define SD_PIN_SCK      10u
#define SD_PIN_MOSI     11u
#define SD_PIN_MISO     12u
#define SD_PIN_CS       13u

#define SPI_FREQ_LOW    400000UL    /* ≤400 kHz required during SD card initialisation */
#define SPI_FREQ_HIGH   25000000UL  /* 25 MHz for normal operation */

static absolute_time_t s_absolute_time_when_timer_on(0UL);
static std::optional<std::size_t> s_timeout_us(0UL);
static bool s_timer_active = false;

void SPI_Init(void) {
    volatile const int actual_baud = spi_init(SD_SPI_INST, SPI_FREQ_LOW);
    spi_set_format(SD_SPI_INST, 8, spi_cpol_t::SPI_CPOL_0, spi_cpha_t::SPI_CPHA_0, spi_order_t::SPI_MSB_FIRST);
    spi_set_slave(SD_SPI_INST, false);

    gpio_set_function(SD_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(SD_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SD_PIN_MISO, GPIO_FUNC_SPI);

    /* CS is driven manually so it stays low across multi-byte commands */
    gpio_init(SD_PIN_CS);
    gpio_set_dir(SD_PIN_CS, GPIO_OUT);
    gpio_put(SD_PIN_CS, 1);
}

BYTE SPI_RW(BYTE d) {
    BYTE rx;
    volatile const int bytes_wr = spi_write_read_blocking(SD_SPI_INST, &d, &rx, 1);
    return rx;
}

void SPI_Release(void) {
    WORD idx;
    for (idx = 512; idx && (SPI_RW(0xFF) != 0xFF); idx--);
}

void SPI_CS_Low(void) {
    gpio_put(SD_PIN_CS, 0);
}

void SPI_CS_High(void) {
    gpio_put(SD_PIN_CS, 1);
}

void SPI_Freq_High(void) {
    spi_set_baudrate(SD_SPI_INST, SPI_FREQ_HIGH);
}

void SPI_Freq_Low(void) {
    spi_set_baudrate(SD_SPI_INST, SPI_FREQ_LOW);
}

void SPI_Timer_On(WORD ms) {
    s_absolute_time_when_timer_on = get_absolute_time();
    s_timeout_us = std::make_optional(ms * 1000UL);
}

BOOL SPI_Timer_Status(void) {
    if (!s_timeout_us.has_value()) {
        return TRUE;
    }
    const auto time_now = get_absolute_time();
    const auto deadline = s_absolute_time_when_timer_on + s_timeout_us.value();
    if (time_now > deadline) {
        return FALSE;
    }
    return TRUE;
}

void SPI_Timer_Off(void) {
    s_timeout_us.reset();
}