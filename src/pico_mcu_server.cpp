#include "pico/stdio.h"
#include "pico/time.h"
#include "pico_gpo.hpp"

#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include <cstdint>
#include <hardware/structs/io_bank0.h>
#include <pico/error.h>
#include "ssd1306.h"


#define LCD_I2C_INSTANCE    i2c_default
#define LCD_I2C_BAUD        1000

pico_mcu_platform::PicoGpo led_gpo(25);

void report_error() {
    led_gpo.set_state(pico_mcu_platform::PicoGpo::State::HIGH);
    while (true) {
        ;
    }
}

int main(void) {
    stdio_init_all();
    led_gpo.set_state(pico_mcu_platform::PicoGpo::State::HIGH);
    sleep_ms(1000);
    led_gpo.set_state(pico_mcu_platform::PicoGpo::State::LOW);
    sleep_ms(1000);
    i2c_init(LCD_I2C_INSTANCE, LCD_I2C_BAUD);
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    const auto addr = static_cast<uint8_t>(0b00111100); // 0b0111101 try this one also 
    const uint8_t buff[] = {
        SSD1306_CMD_START,              // start commands
        SSD1306_DISPLAYOFF,         // turn off display
        SSD1306_SETDISPLAYCLOCKDIV,     // set clock:
        0x80,                           //   Fosc = 8, divide ratio = 0+1
        SSD1306_SETMULTIPLEX,           // display multiplexer:
        (SSD1306_LCDWIDTH - 1),             //   number of display rows
        SSD1306_SETDISPLAYOFFSET,         // display vertical offset:
        0,                              //   no offset
        SSD1306_SETSTARTLINE | 0x00,    // RAM start line 0
        SSD1306_CHARGEPUMP,          // charge pump:
        0x14,                           //   charge pump ON (0x10 for OFF)
        SSD1306_MEMORYMODE,         // addressing mode:
        0x00,                           //   horizontal mode
        SSD1306_COLSCAN_DESCENDING,     // flip columns
        SSD1306_COMSCANINC,      // don't flip rows (pages)
        SSD1306_SETCOMPINS,             // set COM pins
        0x02,                           //   sequential pin mode
        SSD1306_SETCONTRAST,            // set contrast
        0x00,                           //   minimal contrast
        SSD1306_SETPRECHARGE,           // set precharge period
        0xF1,                           //   phase1 = 15, phase2 = 1
        SSD1306_SETVCOMDETECT,           // set VCOMH deselect level
        0x40,                           //   ????? (0,2,3)
        SSD1306_DISPLAYALLON_RESUME,      // use RAM contents for display
        SSD1306_NORMALDISPLAY,          // no inversion
        SSD1306_DEACTIVATE_SCROLL,      // no scrolling
        SSD1306_DISPLAYON,          // turn on display (normal mode)
    };
    if (PICO_ERROR_GENERIC == i2c_write_blocking(LCD_I2C_INSTANCE, addr, buff, sizeof(buff), false)) {
        report_error();
        return 0;
    }
    return 0;
}