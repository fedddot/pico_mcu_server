#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/time.h"

int main(void) {
    const auto ctrl_pin = 14;
    gpio_set_function(ctrl_pin, GPIO_FUNC_PWM);
    const auto slice_num = pwm_gpio_to_slice_num(ctrl_pin);
    const auto channel = pwm_gpio_to_channel(ctrl_pin);
    auto config = pwm_get_default_config();
    pwm_init(slice_num, &config, true);
    const auto level = 32768;
    const auto delay_ms = 500;
    while (true) {
        pwm_set_gpio_level(ctrl_pin, 0);
        sleep_ms(delay_ms);
        pwm_set_gpio_level(ctrl_pin, level);
        sleep_ms(delay_ms);
    }
    return 0;
}