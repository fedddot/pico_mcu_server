#include "ff.h"
#include "diskio.h"

#include "sd_utils.hpp"

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    switch (cmd) {
    case GET_BLOCK_SIZE:
        *(DWORD *)buff = (DWORD)sd_utils::get_block_size();
        return DRESULT::RES_OK;
    case GET_SECTOR_COUNT:
        try {
            const auto sector_count = sd_utils::get_sector_count();
            *(LBA_t *)buff = (LBA_t)sector_count;
            return DRESULT::RES_OK;
        } catch (...) {
            return DRESULT::RES_ERROR;
        }
    case CTRL_SYNC:
        return DRESULT::RES_OK;
    default:
        return DRESULT::RES_ERROR;
    }
}

DSTATUS disk_initialize(BYTE pdrv) {
    (void)pdrv;
    try {
        sd_utils::disk_initialize();
        return 0;
    } catch (...) {
        return -1;
    }
}

DSTATUS disk_status(BYTE pdrv) {
    (void)pdrv;
    if (sd_utils::DiskStatus::READY == sd_utils::disk_status()) {
        return 0;
    }
    return -1;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    (void)pdrv;
    try {
        sd_utils::disk_read(buff, sector, count);
        return DRESULT::RES_OK;
    } catch (...) {
        return DRESULT::RES_ERROR;
    }
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    (void)pdrv;
    try {
        sd_utils::disk_write(buff, sector, count);
        return DRESULT::RES_OK;
    } catch (...) {
        return DRESULT::RES_ERROR;
    }
}

DWORD get_fattime(void) {
    return 0;
}