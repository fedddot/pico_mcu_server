#include <cstddef>
#include <cstring>
#include <optional>
#include <stdexcept>

#include "sd_utils.hpp"
#include "sd_io.h"

using namespace sd_utils;

std::optional<SD_DEV> s_sd_dev(std::nullopt);

void sd_utils::disk_initialize() {
    if (s_sd_dev.has_value()) {
        return;
    }
    SD_DEV dev;
    std::memset(&dev, 0, sizeof(SD_DEV));
    if (SD_OK != SD_Init(&dev)) {
        throw std::runtime_error("Failed to initialize SD card");
    }
    s_sd_dev.emplace(dev);
}

DiskStatus sd_utils::disk_status() {
    if (s_sd_dev.has_value()) {
        return DiskStatus::READY;
    }
    return DiskStatus::NOT_READY;
}

void sd_utils::disk_read(std::uint8_t *dst, const std::size_t sector, const std::size_t count) {
    throw std::runtime_error("Not implemented");
}
void sd_utils::disk_write(const std::uint8_t *src, const std::size_t sector, const std::size_t count) {
    throw std::runtime_error("Not implemented");
}

std::size_t sd_utils::get_block_size() {
    enum: std::size_t {
        BLOCK_SIZE = 512
    };
    return BLOCK_SIZE;
}

std::size_t sd_utils::get_sector_count() {
    if (!s_sd_dev.has_value()) {
        throw std::runtime_error("SD card not initialized");
    }
    return static_cast<std::size_t>(s_sd_dev->last_sector) + 1UL;
}