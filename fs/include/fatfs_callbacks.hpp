#ifndef	FATFS_CALLBACKS_HPP
#define	FATFS_CALLBACKS_HPP

#include "sd_spi_driver.hpp"

namespace fatfs {
    using SpiInit = sdspidriver::SdSpiDriver::SpiInit;
    using SetSpiSpeed = sdspidriver::SdSpiDriver::SetSpiSpeed;
    using TrancieveByte = sdspidriver::SdSpiDriver::TrancieveByte;
    using ChipSelector = sdspidriver::SdSpiDriver::ChipSelector;
    using ChipSelectState = sdspidriver::SdSpiDriver::ChipSelectState;

    void init_fatfs_callbacks(const SpiInit& spi_init, const SetSpiSpeed& set_spi_speed, const TrancieveByte& trancieve_byte, const ChipSelector& chip_selector);
}

#endif // FATFS_CALLBACKS_HPP