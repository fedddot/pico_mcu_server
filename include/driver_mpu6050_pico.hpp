#ifndef	DRIVER_MPU6050_PICO_HPP
#define	DRIVER_MPU6050_PICO_HPP

#include <cmath>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <stdexcept>

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "driver_mpu6050.h"

namespace pico {
    class DriverMpu6050Pico {
    private:
        static constexpr auto I2C_PORT = i2c1;
        static constexpr uint32_t I2C_BAUDRATE = 400000;
        static constexpr uint GPIO_SDA = 2;
        static constexpr uint GPIO_SCL = 3;

    public:
        DriverMpu6050Pico(const mpu6050_address_t iic_addr) {
            DRIVER_MPU6050_LINK_INIT(&m_handle, mpu6050_handle_t);
            DRIVER_MPU6050_LINK_IIC_INIT(&m_handle, iic_init);
            DRIVER_MPU6050_LINK_IIC_DEINIT(&m_handle, iic_deinit);
            DRIVER_MPU6050_LINK_IIC_READ(&m_handle, iic_read);
            DRIVER_MPU6050_LINK_IIC_WRITE(&m_handle, iic_write);
            DRIVER_MPU6050_LINK_DELAY_MS(&m_handle, delay_ms);
            DRIVER_MPU6050_LINK_DEBUG_PRINT(&m_handle, debug_print);
            DRIVER_MPU6050_LINK_RECEIVE_CALLBACK(&m_handle, receive_callback);
            auto res = mpu6050_set_addr_pin(&m_handle, iic_addr);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set addr pin failed.\n");            }
            
            /* init */
            res = mpu6050_init(&m_handle);
            if (res != 0) {
                throw std::runtime_error("mpu6050: init failed.\n");            }
            
            /* disable sleep */
            res = mpu6050_set_sleep(&m_handle, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set sleep failed.\n");            }
            
            /* set the default clock source */
            res = mpu6050_set_clock_source(&m_handle, MPU6050_CLOCK_SOURCE_PLL_X_GYRO);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set clock source failed.\n");            }
            
            /* set the default rate */
            res = mpu6050_set_sample_rate_divider(&m_handle, (1000 / 50) - 1);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set sample rate divider failed.\n");            }
            
            /* set the default low pass filter */
            res = mpu6050_set_low_pass_filter(&m_handle, MPU6050_LOW_PASS_FILTER_3);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set low pass filter failed.\n");            }
            
            /* enable temperature sensor */
            res = mpu6050_set_temperature_sensor(&m_handle, MPU6050_BOOL_TRUE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set temperature sensor failed.\n");            }
            
            /* set the default cycle wake up */
            res = mpu6050_set_cycle_wake_up(&m_handle, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set cycle wake up failed.\n");            }
            
            /* set the default wake up frequency */
            res = mpu6050_set_wake_up_frequency(&m_handle, MPU6050_WAKE_UP_FREQUENCY_1P25_HZ);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set wake up frequency failed.\n");            }
            
            /* enable acc x */
            res = mpu6050_set_standby_mode(&m_handle, MPU6050_SOURCE_ACC_X, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set standby mode failed.\n");
            }
            
            /* enable acc y */
            res = mpu6050_set_standby_mode(&m_handle, MPU6050_SOURCE_ACC_Y, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set standby mode failed.\n");
            }
            
            /* enable acc z */
            res = mpu6050_set_standby_mode(&m_handle, MPU6050_SOURCE_ACC_Z, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set standby mode failed.\n");
            }
            
            /* enable gyro x */
            res = mpu6050_set_standby_mode(&m_handle, MPU6050_SOURCE_GYRO_X, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set standby mode failed.\n");
            }
            
            /* enable gyro y */
            res = mpu6050_set_standby_mode(&m_handle, MPU6050_SOURCE_GYRO_Y, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set standby mode failed.\n");
            }
            
            /* enable gyro z */
            res = mpu6050_set_standby_mode(&m_handle, MPU6050_SOURCE_GYRO_Z, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set standby mode failed.\n");
            }
            
            /* disable gyroscope x test */
            res = mpu6050_set_gyroscope_test(&m_handle, MPU6050_AXIS_X, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set gyroscope test failed.\n");
            }
            
            /* disable gyroscope y test */
            res = mpu6050_set_gyroscope_test(&m_handle, MPU6050_AXIS_Y, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set gyroscope test failed.\n");
            }
            
            /* disable gyroscope z test */
            res = mpu6050_set_gyroscope_test(&m_handle, MPU6050_AXIS_Z, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set gyroscope test failed.\n");
            }
            
            /* disable accelerometer x test */
            res = mpu6050_set_accelerometer_test(&m_handle, MPU6050_AXIS_X, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set accelerometer test failed.\n");
            }
            
            /* disable accelerometer y test */
            res = mpu6050_set_accelerometer_test(&m_handle, MPU6050_AXIS_Y, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set accelerometer test failed.\n");
            }
            
            /* disable accelerometer z test */
            res = mpu6050_set_accelerometer_test(&m_handle, MPU6050_AXIS_Z, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set accelerometer test failed.\n");
            }
            
            /* disable fifo */
            res = mpu6050_set_fifo(&m_handle, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set fifo failed.\n");
            }
            
            /* disable temp fifo */
            res = mpu6050_set_fifo_enable(&m_handle, MPU6050_FIFO_TEMP, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set fifo enable failed.\n");
            }
            
            /* disable xg fifo */
            res = mpu6050_set_fifo_enable(&m_handle, MPU6050_FIFO_XG, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set fifo enable failed.\n");
            }
            
            /* disable yg fifo */
            res = mpu6050_set_fifo_enable(&m_handle, MPU6050_FIFO_YG, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set fifo enable failed.\n");
            }
            
            /* disable zg fifo */
            res = mpu6050_set_fifo_enable(&m_handle, MPU6050_FIFO_ZG, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set fifo enable failed.\n");
            }
            
            /* disable accel fifo */
            res = mpu6050_set_fifo_enable(&m_handle, MPU6050_FIFO_ACCEL, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set fifo enable failed.\n");
            }
            
            /* set the default interrupt level */
            res = mpu6050_set_interrupt_level(&m_handle, MPU6050_PIN_LEVEL_LOW);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set interrupt level failed.\n");
            }
            
            /* set the default pin type */
            res = mpu6050_set_interrupt_pin_type(&m_handle, MPU6050_PIN_TYPE_PUSH_PULL);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set interrupt pin type failed.\n");
            }
            
            /* set the default motion interrupt */
            res = mpu6050_set_interrupt(&m_handle, MPU6050_INTERRUPT_MOTION, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set interrupt failed.\n");
            }
            
            /* set the default fifo overflow interrupt */
            res = mpu6050_set_interrupt(&m_handle, MPU6050_INTERRUPT_FIFO_OVERFLOW, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set interrupt failed.\n");
            }
            
            /* set the default dmp interrupt */
            res = mpu6050_set_interrupt(&m_handle, MPU6050_INTERRUPT_DMP, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set interrupt failed.\n");
            }
            
            /* set the default i2c master interrupt */
            res = mpu6050_set_interrupt(&m_handle, MPU6050_INTERRUPT_I2C_MAST, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set interrupt failed.\n");
            }
            
            /* set the default data ready interrupt */
            res = mpu6050_set_interrupt(&m_handle, MPU6050_INTERRUPT_DATA_READY, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set interrupt failed.\n");
            }
            
            /* set the default interrupt latch */
            res = mpu6050_set_interrupt_latch(&m_handle, MPU6050_BOOL_TRUE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set interrupt latch failed.\n");
            }
            
            /* set the default interrupt read clear */
            res = mpu6050_set_interrupt_read_clear(&m_handle, MPU6050_BOOL_TRUE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set interrupt read clear failed.\n");
            }
            
            /* set the extern sync */
            res = mpu6050_set_extern_sync(&m_handle, MPU6050_EXTERN_SYNC_INPUT_DISABLED);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set extern sync failed.\n");
            }
            
            /* set the default fsync interrupt */
            res = mpu6050_set_fsync_interrupt(&m_handle, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set fsync interrupt failed.\n");
            }
            
            /* set the default fsync interrupt level */
            res = mpu6050_set_fsync_interrupt_level(&m_handle, MPU6050_PIN_LEVEL_LOW);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set fsync interrupt level failed.\n");
            }
            
            /* set the default iic master */
            res = mpu6050_set_iic_master(&m_handle, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set iic master failed.\n");
            }
            
            /* set the default iic bypass */
            res = mpu6050_set_iic_bypass(&m_handle, MPU6050_BOOL_FALSE);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set iic bypass failed.\n");
            }
            
            /* set the default accelerometer range */
            res = mpu6050_set_accelerometer_range(&m_handle, MPU6050_ACCELEROMETER_RANGE_2G);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set accelerometer range failed.\n");
            }
            
            /* set the default gyroscope range */
            res = mpu6050_set_gyroscope_range(&m_handle, MPU6050_GYROSCOPE_RANGE_2000DPS);
            if (res != 0) {
                throw std::runtime_error("mpu6050: set gyroscope range failed.\n");
            }
        }
        DriverMpu6050Pico(const DriverMpu6050Pico&) = delete;
        DriverMpu6050Pico& operator=(const DriverMpu6050Pico&) = delete;
        ~DriverMpu6050Pico() noexcept = default;

        std::float_t read_temp() {
            std::int16_t raw_value;
            std::float_t degree_value;
            if (0 != mpu6050_read_temperature(&m_handle, &raw_value, &degree_value)) {
                throw std::runtime_error("Failed to read temperature from MPU6050");            }
            return degree_value;
        }
        
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
                return 1;            }
            if (i2c_read_blocking(I2C_PORT, device_addr, buf, len, false) == PICO_ERROR_GENERIC) {
                return 1;            }
            return 0;
        }

        static uint8_t iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
            uint8_t device_addr = addr >> 1;
            uint8_t write_buffer[len + 1];
            write_buffer[0] = reg;
            std::memcpy(&write_buffer[1], buf, len);
            if (i2c_write_blocking(I2C_PORT, device_addr, write_buffer, len + 1, false) == PICO_ERROR_GENERIC) {
                return 1;            }
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
                    break;            }
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
                    break;            }
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
                    break;            }
        }
    };
}

#endif // DRIVER_MPU6050_PICO_HPP