#include <cstdint>
#include <optional>

#include "ff.h"
#include "diskio.h"

#include "sd_spi_driver.hpp"
#include "fatfs_callbacks.hpp"

using namespace sdspidriver;
using namespace fatfs;

extern std::optional<SdSpiDriver> s_sd_driver;

void fatfs::init_fatfs_callbacks(const SpiInit& spi_init, const SetSpiSpeed& set_spi_speed, const TrancieveByte& trancieve_byte, const ChipSelector& chip_selector) {
    s_sd_driver.emplace(spi_init, set_spi_speed, trancieve_byte, chip_selector);
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    switch (cmd) {
    case GET_BLOCK_SIZE:
        *(DWORD *)buff = (DWORD)SdSpiDriver::BLOCK_SIZE;
        return DRESULT::RES_OK;
    case GET_SECTOR_COUNT:
        if (!s_sd_driver.has_value()) {
            return DRESULT::RES_ERROR;
        }
        *(LBA_t *)buff = (LBA_t)s_sd_driver->total_blocks();
        return DRESULT::RES_OK;
    case CTRL_SYNC:
        return DRESULT::RES_OK;
    default:
        return DRESULT::RES_ERROR;
    }
}

DSTATUS disk_initialize(BYTE pdrv) {
    return disk_status(pdrv);
}

DSTATUS disk_status(BYTE pdrv) {
    (void)pdrv;
    if (!s_sd_driver.has_value()) {
        return -1;
    }
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    try {
        if (disk_status(pdrv) != 0) {
            return DRESULT::RES_ERROR;
        }
        for (std::uint32_t i = 0; i < count; ++i) {
            const auto block_data = s_sd_driver->read_block(sector + i);
            std::copy(block_data.begin(), block_data.end(), buff + i * SdSpiDriver::BLOCK_SIZE);
        }
        return DRESULT::RES_OK;
    } catch (...) {
        return DRESULT::RES_ERROR;
    }
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    try {
        if (disk_status(pdrv) != 0) {
            return DRESULT::RES_ERROR;
        }
        for (std::uint32_t i = 0; i < count; ++i) {
            std::array<std::uint8_t, SdSpiDriver::BLOCK_SIZE> block_data;
            std::copy(buff + i * SdSpiDriver::BLOCK_SIZE, buff + i * SdSpiDriver::BLOCK_SIZE + SdSpiDriver::BLOCK_SIZE, block_data.begin());
            s_sd_driver->write_block(sector + i, block_data);
        }
        return DRESULT::RES_OK;
    } catch (...) {
        return DRESULT::RES_ERROR;
    }
}

DWORD get_fattime(void) {
    return 0;
}