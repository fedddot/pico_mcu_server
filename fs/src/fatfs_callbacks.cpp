#include <cstring>

#include "ff.h"
#include "diskio.h"
#include "disk.h"

#define DISK_SIZE _usr_app_src_fs_src_disk_img_len
#define BLOCK_SIZE 0x80UL
#define SECTOR_COUNT (DISK_SIZE / BLOCK_SIZE)

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    switch (cmd) {
    case GET_BLOCK_SIZE:
        *(DWORD *)buff = (DWORD)BLOCK_SIZE;
        return DRESULT::RES_OK;
    case GET_SECTOR_COUNT:
        *(LBA_t *)buff = (LBA_t)SECTOR_COUNT;
        return DRESULT::RES_OK;
    case CTRL_SYNC:
        return DRESULT::RES_OK;
    default:
        return DRESULT::RES_ERROR;
    }
}

DSTATUS disk_initialize(BYTE pdrv) {
    return 0;
}

DSTATUS disk_status(BYTE pdrv) {
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    enum { SECTOR_SIZE = 512 };
    std::memcpy(buff, _usr_app_src_fs_src_disk_img + sector * SECTOR_SIZE, count * SECTOR_SIZE);
    return DRESULT::RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    enum { SECTOR_SIZE = 512 };
    std::memcpy(_usr_app_src_fs_src_disk_img + sector * SECTOR_SIZE, buff, count * SECTOR_SIZE);
    return DRESULT::RES_OK;
}

DWORD get_fattime(void) {
    return 0;
}