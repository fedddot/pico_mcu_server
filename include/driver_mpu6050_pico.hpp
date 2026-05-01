#ifndef	DRIVER_MPU6050_PICO_HPP
#define	DRIVER_MPU6050_PICO_HPP

#include <cstdint>

#include "driver_mpu6050.h"

namespace pico {
    class DriverMpu6050Pico {
    public:
        DriverMpu6050Pico(const uint8_t iic_addr) {
            mpu6050_handle_t handle;
            DRIVER_MPU6050_LINK_INIT(&handle, mpu6050_handle_t);
            DRIVER_MPU6050_LINK_IIC_INIT(&handle, iic_init);
            DRIVER_MPU6050_LINK_IIC_DEINIT(&handle, iic_deinit);
            DRIVER_MPU6050_LINK_IIC_READ(&handle, iic_read);
            DRIVER_MPU6050_LINK_IIC_WRITE(&handle, iic_write);
            DRIVER_MPU6050_LINK_DELAY_MS(&handle, delay_ms);
            DRIVER_MPU6050_LINK_DEBUG_PRINT(&handle, debug_print);
            DRIVER_MPU6050_LINK_RECEIVE_CALLBACK(&handle, receive_callback);

        }
        DriverMpu6050Pico(const DriverMpu6050Pico&) = delete;
        DriverMpu6050Pico& operator=(const DriverMpu6050Pico&) = delete;
        ~DriverMpu6050Pico() noexcept = default;
        
    private:
        static uint8_t iic_init(void);
        static uint8_t iic_deinit(void);
        static uint8_t iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
        static uint8_t iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
        static void delay_ms(uint32_t ms);
        static void debug_print(const char *const fmt, ...);
        static void receive_callback(uint8_t type);
        static void dmp_tap_callback(uint8_t count, uint8_t direction);
        static void dmp_orient_callback(uint8_t orientation);
    };
}

#endif // DRIVER_MPU6050_PICO_HPP