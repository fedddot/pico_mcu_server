#include <cstring>
#include <stdexcept>

#include "ff.h"

PARTITION VolToPart[] = {
    {0, 0},
};

int main(void) {    
    FATFS fs;
    std::memset(&fs, 0, sizeof(fs));
    if (FRESULT::FR_OK != f_mount(&fs, "0:", 1)) {
        throw std::runtime_error("Failed to mount SD card");
    }

    FIL file;
    if (FRESULT::FR_OK != f_open(&file, "0:TEST.MD", FA_CREATE_ALWAYS | FA_WRITE)) {
        throw std::runtime_error("Failed to open file on SD card");
    }
    char data[] = "onetwothree";
    UINT bytes_write(0);
    if (FRESULT::FR_OK != f_write(&file, data, sizeof(data), &bytes_write)) {
        throw std::runtime_error("Failed to read from file on SD card");
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
