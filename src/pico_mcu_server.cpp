#include <cstring>

extern "C" {
    #include "sd_io.h"
}

int main(void) {
    SD_DEV sd_dev;
    std::memset(&sd_dev, 0, sizeof(sd_dev));
    volatile const auto status = SD_Init(&sd_dev);
    if (SD_OK != status) {
        return -1;
    }
    while (true) {
    }
    return 0;
}
