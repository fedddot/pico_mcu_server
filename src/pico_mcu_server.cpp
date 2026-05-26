#include <cstring>
#include <stdexcept>

#include "ff.h"

int main(void) {
    FATFS fs;
    std::memset(&fs, 0, sizeof(fs));
    volatile auto disk_status = f_mount(&fs, "0:", 1);
    if (FRESULT::FR_OK != disk_status) {
        throw std::runtime_error("Failed to mount SD card");
    }

    FIL file;
    volatile auto open_res = f_open(&file, "0:NEW_FILE", FA_READ);

    char data[128] = { '\0' };
    UINT rw_size(0);
    if (FRESULT::FR_OK != f_read(&file, data, sizeof(data), &rw_size)) {
        throw std::runtime_error("Failed to write to file on SD card");
    }
    if (FRESULT::FR_OK != f_close(&file)) {
        throw std::runtime_error("Failed to close file on SD card");
    }
    if (FRESULT::FR_OK != f_unmount("0:")) {
        throw std::runtime_error("Failed to unmount SD card");
    }

    while (true) {
        // Loop forever
    }
    return 0;
}
