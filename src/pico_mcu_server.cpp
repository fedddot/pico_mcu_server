#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/time.h"

#include "sd_spi_driver.hpp"

#define BLOCK_SIZE 512UL

using namespace sd_spi_driver;

static void sd_spi_init(void);
static void sd_set_spi_speed(const SdSpiDriver<BLOCK_SIZE>::SpiSpeed speed);
static std::uint8_t sd_trancieve_byte(const std::uint8_t byte);
static void sd_delay(const std::size_t ms);
static void sd_chip_selector(const SdSpiDriver<BLOCK_SIZE>::ChipSelectState state);

int main(void) {
    SdSpiDriver<BLOCK_SIZE> sd_driver(
        sd_spi_init,
        sd_set_spi_speed,
        sd_trancieve_byte,
        sd_delay,
        sd_chip_selector
    );
    while (true) {
        // Loop forever
    }
    return 0;
}

#define SD_SPI_INST     spi1
#define SD_PIN_SCK      10u
#define SD_PIN_MOSI     11u
#define SD_PIN_MISO     12u
#define SD_PIN_CS       13u

void sd_spi_init(void) {
    volatile const int actual_baud = spi_init(
        SD_SPI_INST,
        static_cast<uint>(SdSpiDriver<BLOCK_SIZE>::SpiSpeed::LOW_SPEED)
    );
    spi_set_format(SD_SPI_INST, 8, spi_cpol_t::SPI_CPOL_0, spi_cpha_t::SPI_CPHA_0, spi_order_t::SPI_MSB_FIRST);
    spi_set_slave(SD_SPI_INST, false);

    gpio_set_function(SD_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(SD_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SD_PIN_MISO, GPIO_FUNC_SPI);

    gpio_init(SD_PIN_CS);
    gpio_set_dir(SD_PIN_CS, GPIO_OUT);
    gpio_put(SD_PIN_CS, 1);
}

void sd_set_spi_speed(const SdSpiDriver<BLOCK_SIZE>::SpiSpeed speed) {
    spi_set_baudrate(SD_SPI_INST, static_cast<uint>(speed));
}

std::uint8_t sd_trancieve_byte(const std::uint8_t byte) {
    std::uint8_t rx;
    volatile const int bytes_wr = spi_write_read_blocking(SD_SPI_INST, &byte, &rx, 1);
    return rx;
}

void sd_delay(const std::size_t ms) {
    sleep_ms(ms);
}

void sd_chip_selector(const SdSpiDriver<BLOCK_SIZE>::ChipSelectState state) {
    switch (state) {
    case SdSpiDriver<BLOCK_SIZE>::ChipSelectState::SELECTED:
        gpio_put(SD_PIN_CS, 0);
        break;
    case SdSpiDriver<BLOCK_SIZE>::ChipSelectState::UNSELECTED:
        gpio_put(SD_PIN_CS, 1);
        break;
    default:
        throw std::invalid_argument("invalid ChipSelectState provided to sd_chip_selector");
    }
}