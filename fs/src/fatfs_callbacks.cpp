#include <array>
#include <cstring>
#include <fstream>

#include "gtest/gtest.h"

#include "custom_ffconf.h"
#include "ff.h"
#include "diskio.h"

#define BLOCK_SIZE 0x200UL
#define SECTOR_COUNT 0x800UL
#define DISK_SIZE (BLOCK_SIZE * SECTOR_COUNT)

std::array<char, DISK_SIZE> g_disk;

TEST(ut_fatfs, fatfs_sanity) {
    const auto fs_path = "0";
    MKFS_PARM mkfs_parm = {
        .fmt = FM_FAT,
        .n_fat = 1
    };
    std::array<BYTE, FF_MAX_SS> work;

    auto fs_result = f_mkfs(fs_path, &mkfs_parm, work.data(), work.size());
    ASSERT_EQ(FRESULT::FR_OK, fs_result);
    
    FATFS fs;
    fs_result = f_mount(&fs, fs_path, 0);
    ASSERT_EQ(FRESULT::FR_OK, fs_result);

    const auto dir_path = "work_dir";
    fs_result = f_mkdir(dir_path);
    ASSERT_EQ(FRESULT::FR_OK, fs_result);
    
    FIL file; 
    const auto file_path = "work_dir/test.txt";
    fs_result = f_open(&file, file_path, FA_WRITE | FA_CREATE_NEW);
    ASSERT_EQ(FRESULT::FR_OK, fs_result);

    const auto data = "UPDATED_FILE";
    UINT bytes_written;
    fs_result = f_write(&file, data, std::strlen(data), &bytes_written);
    ASSERT_EQ(FRESULT::FR_OK, fs_result);
    ASSERT_EQ(std::strlen(data), bytes_written);

    fs_result = f_close(&file);
    ASSERT_EQ(FRESULT::FR_OK, fs_result);
    
    fs_result = f_unmount(fs_path);
    ASSERT_EQ(FRESULT::FR_OK, fs_result);

    std::ofstream disk_image("disk.img", std::ios::binary);
    ASSERT_TRUE(disk_image.is_open());
    disk_image.write(g_disk.data(), g_disk.size());
    ASSERT_TRUE(disk_image.good());
}

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
    std::memcpy(buff, g_disk.data() + sector * SECTOR_SIZE, count * SECTOR_SIZE);
    return DRESULT::RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    enum { SECTOR_SIZE = 512 };
    std::memcpy(g_disk.data() + sector * SECTOR_SIZE, buff, count * SECTOR_SIZE);
    return DRESULT::RES_OK;
}

DWORD get_fattime(void) {
    return 0;
}