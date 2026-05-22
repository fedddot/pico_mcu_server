#ifndef SD_UTILS_HPP
#define SD_UTILS_HPP

#include <cstddef>
#include <cstdint>

namespace sd_utils {
    enum class DiskStatus: int {
        READY,
        NOT_READY
    };
    void disk_initialize();
    DiskStatus disk_status();
    void disk_read(std::uint8_t *dst, const std::size_t sector, const std::size_t count);
    void disk_write(const std::uint8_t *src, const std::size_t sector, const std::size_t count);
    std::size_t get_block_size();
    std::size_t get_sector_count();
} // namespace sd_utils

#endif // SD_UTILS_HPP