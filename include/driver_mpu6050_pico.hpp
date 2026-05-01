#ifndef	DRIVER_MPU6050_PICO_HPP
#define	DRIVER_MPU6050_PICO_HPP

#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cstdio>

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "driver_mpu6050.h"

namespace pico {
    class DriverMpu6050Pico {
    private:
        static constexpr auto I2C_PORT = i2c1;
        static constexpr uint32_t I2C_BAUDRATE = 400000;
        static constexpr uint GPIO_SDA = 26;
        static constexpr uint GPIO_SCL = 27;

    public:
        DriverMpu6050Pico(const uint8_t iic_addr) {
            DRIVER_MPU6050_LINK_INIT(&m_handle, mpu6050_handle_t);
            DRIVER_MPU6050_LINK_IIC_INIT(&m_handle, iic_init);
            DRIVER_MPU6050_LINK_IIC_DEINIT(&m_handle, iic_deinit);
            DRIVER_MPU6050_LINK_IIC_READ(&m_handle, iic_read);
            DRIVER_MPU6050_LINK_IIC_WRITE(&m_handle, iic_write);
            DRIVER_MPU6050_LINK_DELAY_MS(&m_handle, delay_ms);
            DRIVER_MPU6050_LINK_DEBUG_PRINT(&m_handle, debug_print);
            DRIVER_MPU6050_LINK_RECEIVE_CALLBACK(&m_handle, receive_callback);
        }
        DriverMpu6050Pico(const DriverMpu6050Pico&) = delete;
        DriverMpu6050Pico& operator=(const DriverMpu6050Pico&) = delete;
        ~DriverMpu6050Pico() noexcept = default;
        
    private:
        mpu6050_handle_t m_handle;
        static uint8_t iic_init(void) {
            i2c_init(I2C_PORT, I2C_BAUDRATE);
            gpio_set_function(GPIO_SDA, GPIO_FUNC_I2C);
            gpio_set_function(GPIO_SCL, GPIO_FUNC_I2C);
            gpio_pull_up(GPIO_SDA);
            gpio_pull_up(GPIO_SCL);
            return 0;
        }

        static uint8_t iic_deinit(void) {
            i2c_deinit(I2C_PORT);
            return 0;
        }

        static uint8_t iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
            uint8_t device_addr = addr >> 1;
            if (i2c_write_blocking(I2C_PORT, device_addr, &reg, 1, true) == PICO_ERROR_GENERIC) {
                return 1;
            }
            if (i2c_read_blocking(I2C_PORT, device_addr, buf, len, false) == PICO_ERROR_GENERIC) {
                return 1;
            }
            return 0;
        }

        static uint8_t iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
            uint8_t device_addr = addr >> 1;
            uint8_t write_buffer[len + 1];
            write_buffer[0] = reg;
            std::memcpy(&write_buffer[1], buf, len);
            if (i2c_write_blocking(I2C_PORT, device_addr, write_buffer, len + 1, false) == PICO_ERROR_GENERIC) {
                return 1;
            }
            return 0;
        }

        static void delay_ms(uint32_t ms) {
            sleep_ms(ms);
        }

        static void debug_print(const char *const fmt, ...) {
            char str[256];
            va_list args;
            std::memset(str, 0, sizeof(str));
            va_start(args, fmt);
            vsnprintf(str, sizeof(str) - 1, fmt, args);
            va_end(args);
            printf("%s", str);
        }

        static void receive_callback(uint8_t type) {
            switch (type) {
                case MPU6050_INTERRUPT_MOTION:
                    debug_print("mpu6050: irq motion.\n");
                    break;
                case MPU6050_INTERRUPT_FIFO_OVERFLOW:
                    debug_print("mpu6050: irq fifo overflow.\n");
                    break;
                case MPU6050_INTERRUPT_I2C_MAST:
                    debug_print("mpu6050: irq i2c master.\n");
                    break;
                case MPU6050_INTERRUPT_DMP:
                    debug_print("mpu6050: irq dmp.\n");
                    break;
                case MPU6050_INTERRUPT_DATA_READY:
                    debug_print("mpu6050: irq data ready.\n");
                    break;
                default:
                    debug_print("mpu6050: irq unknown code.\n");
                    break;
            }
        }

        static void dmp_tap_callback(uint8_t count, uint8_t direction) {
            switch (direction) {
                case MPU6050_DMP_TAP_X_UP:
                    debug_print("mpu6050: tap irq x up with %d.\n", count);
                    break;
                case MPU6050_DMP_TAP_X_DOWN:
                    debug_print("mpu6050: tap irq x down with %d.\n", count);
                    break;
                case MPU6050_DMP_TAP_Y_UP:
                    debug_print("mpu6050: tap irq y up with %d.\n", count);
                    break;
                case MPU6050_DMP_TAP_Y_DOWN:
                    debug_print("mpu6050: tap irq y down with %d.\n", count);
                    break;
                case MPU6050_DMP_TAP_Z_UP:
                    debug_print("mpu6050: tap irq z up with %d.\n", count);
                    break;
                case MPU6050_DMP_TAP_Z_DOWN:
                    debug_print("mpu6050: tap irq z down with %d.\n", count);
                    break;
                default:
                    debug_print("mpu6050: tap irq unknown code.\n");
                    break;
            }
        }

        static void dmp_orient_callback(uint8_t orientation) {
            switch (orientation) {
                case MPU6050_DMP_ORIENT_PORTRAIT:
                    debug_print("mpu6050: orient irq portrait.\n");
                    break;
                case MPU6050_DMP_ORIENT_LANDSCAPE:
                    debug_print("mpu6050: orient irq landscape.\n");
                    break;
                case MPU6050_DMP_ORIENT_REVERSE_PORTRAIT:
                    debug_print("mpu6050: orient irq reverse portrait.\n");
                    break;
                case MPU6050_DMP_ORIENT_REVERSE_LANDSCAPE:
                    debug_print("mpu6050: orient irq reverse landscape.\n");
                    break;
                default:
                    debug_print("mpu6050: orient irq unknown code.\n");
                    break;
            }
        }
    };
}

#endif // DRIVER_MPU6050_PICO_HPP