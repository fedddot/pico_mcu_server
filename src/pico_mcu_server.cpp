#include <cstring>

#include "ff.h"

int main(void) {
    FATFS fs;
    std::memset(&fs, 0, sizeof(fs));

    if (FRESULT::FR_OK != f_mount(&fs, "0", 1)) {
        return -1;
    }

    FIL file;
    if (FRESULT::FR_OK != f_open(&file, "test", FA_WRITE | FA_CREATE_ALWAYS)) {
        return -1;
    }
    const auto data = "Hello, World!";
    UINT bytes_written(0);
    if (FRESULT::FR_OK != f_write(&file, data, std::strlen(data), &bytes_written)) {
        return -1;
    }
    if (FRESULT::FR_OK != f_close(&file)) {
        return -1;
    }

    while (true) {
    }
    return 0;
}
