#ifndef	DRIVER_MPU6050_PICO_HPP
#define	DRIVER_MPU6050_PICO_HPP

#include <map>

#include "driver_mpu6050.h"

namespace pico {
    class DriverMpu6050Pico {
    public:
        DriverMpu6050Pico();
        DriverMpu6050Pico(const DriverMpu6050Pico&) = delete;
        DriverMpu6050Pico& operator=(const DriverMpu6050Pico&) = delete;
        ~DriverMpu6050Pico() noexcept override;
        
    private:


    };
}

#endif // DRIVER_MPU6050_PICO_HPP