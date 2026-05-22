#include <cstddef>
#include <cstring>
#include <optional>
#include <stdexcept>

#include "sd_utils.hpp"
extern "C" {
#include "sd_io.h"
}

using namespace sd_utils;

enum: std::size_t {
    BLOCK_SIZE = 512
};

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
    if (!s_sd_dev.has_value()) {
        throw std::runtime_error("SD card not initialized");
    }
    for (std::size_t i = 0; i < count; ++i) {
        if (SD_OK != SD_Read(&s_sd_dev.value(), dst + i * BLOCK_SIZE, sector + i, 0, BLOCK_SIZE)) {
            throw std::runtime_error("Failed to read from SD card");
        }
    }
}
void sd_utils::disk_write(const std::uint8_t *src, const std::size_t sector, const std::size_t count) {
    if (!s_sd_dev.has_value()) {
        throw std::runtime_error("SD card not initialized");
    }
    for (std::size_t i = 0; i < count; ++i) {
        if (SD_OK != SD_Write(&s_sd_dev.value(), const_cast<std::uint8_t *>(src + i * BLOCK_SIZE), sector + i)) {
            throw std::runtime_error("Failed to read from SD card");
        }
    }
}

std::size_t sd_utils::get_block_size() {
    return BLOCK_SIZE;
}

std::size_t sd_utils::get_sector_count() {
    if (!s_sd_dev.has_value()) {
        throw std::runtime_error("SD card not initialized");
    }
    return static_cast<std::size_t>(s_sd_dev->last_sector) + 1UL;
}