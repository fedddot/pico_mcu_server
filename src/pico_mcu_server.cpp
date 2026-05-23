#include <cstring>
#include <stdexcept>

#include "ff.h"

int main(void) {
    FATFS fs;
    std::memset(&fs, 0, sizeof(fs));

    if (FRESULT::FR_OK != f_mount(&fs, "0:", 0)) {
        throw std::runtime_error("Failed to mount SD card");
    }

    FIL file;
    if (FRESULT::FR_OK != f_open(&file, "0:test.md", FA_READ)) {
        throw std::runtime_error("Failed to open file on SD card");
    }
    char data[20UL] = { '\0' };
    UINT bytes_read(0);
    if (FRESULT::FR_OK != f_read(&file, data, sizeof(data), &bytes_read)) {
        throw std::runtime_error("Failed to read from file on SD card");
    }
    if (FRESULT::FR_OK != f_close(&file)) {
        throw std::runtime_error("Failed to close file on SD card");
    }

    while (true) {
        // Loop forever
    }
    return 0;
}
