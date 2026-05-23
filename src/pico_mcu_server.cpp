#include <cstring>

#include "ff.h"

int main(void) {
    FATFS fs;
    std::memset(&fs, 0, sizeof(fs));

    if (FRESULT::FR_OK != f_mount(&fs, "0", 1)) {
        return -1;
    }

    FIL file;
    if (FRESULT::FR_OK != f_open(&file, "test.md", FA_READ)) {
        return -1;
    }
    char data[20UL] = { '\0' };
    UINT bytes_read(0);
    if (FRESULT::FR_OK != f_read(&file, data, sizeof(data), &bytes_read)) {
        return -1;
    }
    if (FRESULT::FR_OK != f_close(&file)) {
        return -1;
    }

    while (true) {
    }
    return 0;
}
