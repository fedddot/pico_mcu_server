#include <cstring>

#include "ff.h"
#include "diskio.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/config.h"

// __flash_binary_end is provided by the linker (memmap_default.ld, .flash_end section).
// It marks the byte immediately after the last byte of the firmware binary in flash.
extern char __flash_binary_end;

// Filesystem occupies the flash region between the end of the firmware and the end
// of flash. The start is rounded up to a FLASH_SECTOR_SIZE (4096-byte) boundary so
// that the first erase operation is always aligned.
static const uint32_t FS_FLASH_OFFSET =
    (reinterpret_cast<uint32_t>(&__flash_binary_end) - XIP_BASE + FLASH_SECTOR_SIZE - 1)
    & ~static_cast<uint32_t>(FLASH_SECTOR_SIZE - 1);

static const uint32_t FLASH_END_OFFSET = PICO_FLASH_SIZE_BYTES; // offset from XIP_BASE

#define SECTOR_SIZE 512U
#define DISK_SIZE   (FLASH_END_OFFSET - FS_FLASH_OFFSET)
#define BLOCK_SIZE  (FLASH_SECTOR_SIZE / SECTOR_SIZE)  // erase block in FAT sectors (= 8)
#define SECTOR_COUNT (DISK_SIZE / SECTOR_SIZE)

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
    // Flash is memory-mapped at XIP_BASE; read directly
    const uint32_t src = XIP_BASE + FS_FLASH_OFFSET + sector * SECTOR_SIZE;
    std::memcpy(buff, reinterpret_cast<const uint8_t *>(src), count * SECTOR_SIZE);
    return DRESULT::RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    const uint32_t write_offset = sector * SECTOR_SIZE;
    const uint32_t write_size   = count  * SECTOR_SIZE;

    // Flash erase granularity is FLASH_SECTOR_SIZE (4096 bytes).
    const uint32_t first_flash_sector = (FS_FLASH_OFFSET + write_offset) / FLASH_SECTOR_SIZE;
    const uint32_t last_flash_sector  = (FS_FLASH_OFFSET + write_offset + write_size - 1) / FLASH_SECTOR_SIZE;

    uint8_t sector_buf[FLASH_SECTOR_SIZE];

    for (uint32_t fs_idx = first_flash_sector; fs_idx <= last_flash_sector; fs_idx++) {
        const uint32_t flash_sector_start = fs_idx * FLASH_SECTOR_SIZE;
        const uint32_t flash_sector_end   = flash_sector_start + FLASH_SECTOR_SIZE;

        // Read the full 4 KB flash sector into RAM so unrelated bytes are preserved
        std::memcpy(sector_buf,
                    reinterpret_cast<const uint8_t *>(XIP_BASE + flash_sector_start),
                    FLASH_SECTOR_SIZE);

        // Compute overlap between this flash sector and the write range
        const uint32_t abs_write_start = FS_FLASH_OFFSET + write_offset;
        const uint32_t abs_write_end   = abs_write_start + write_size;

        const uint32_t overlap_start = abs_write_start > flash_sector_start ? abs_write_start : flash_sector_start;
        const uint32_t overlap_end   = abs_write_end   < flash_sector_end   ? abs_write_end   : flash_sector_end;

        // Patch the sector buffer with the incoming data
        std::memcpy(sector_buf + (overlap_start - flash_sector_start),
                    buff        + (overlap_start - abs_write_start),
                    overlap_end - overlap_start);

        // Erase and reprogram with interrupts disabled to prevent any flash
        // access from interrupt handlers while XIP is temporarily suspended
        const uint32_t saved_ints = save_and_disable_interrupts();
        flash_range_erase(flash_sector_start, FLASH_SECTOR_SIZE);
        flash_range_program(flash_sector_start, sector_buf, FLASH_SECTOR_SIZE);
        restore_interrupts(saved_ints);
    }

    return DRESULT::RES_OK;
}

DWORD get_fattime(void) {
    return 0;
}