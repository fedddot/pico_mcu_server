#include <cstring>

extern "C" {
    #include "sd_io.h"
}

int main(void) {
    SD_DEV sd_dev;
    std::memset(&sd_dev, 0, sizeof(sd_dev));
    volatile const auto init_status = SD_Init(&sd_dev);
    if (SD_OK != init_status) {
        return -1;
    }
    char buf[512];
    volatile auto res = SD_Read(&sd_dev, buf, 0, 0, 512);
    res = SD_Read(&sd_dev, buf, 462, 0, 512);
    res = SD_Read(&sd_dev, buf, 463, 0, 512);
    res = SD_Read(&sd_dev, buf, 464, 0, 512);
    res = SD_Read(&sd_dev, buf, 465, 0, 512);
    while (true) {
    }
    return 0;
}
