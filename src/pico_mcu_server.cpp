#include <cstring>
#include "ff.h"
int main(void) {
    FATFS fs;
    std::memset(&fs, 0, sizeof(fs));

    if (FRESULT::FR_OK != f_mount(&fs, "0", 0)) {
        return -1;
    }
    
    while (true) {
    }
    return 0;
}
