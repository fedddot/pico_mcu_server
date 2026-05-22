#include <cstring>
#include "ff.h"
int main(void) {
    FATFS fs;
    std::memset(&fs, 0, sizeof(fs));

    f_mount(&fs, "0", 0);   
    
    while (true) {
    }
    return 0;
}
