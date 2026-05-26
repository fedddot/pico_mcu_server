#ifndef	SD_SPI_DRIVER_HPP
#define	SD_SPI_DRIVER_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>

namespace sd_spi_driver {
    template <std::size_t BlockSize = 512UL>
    class SdSpiDriver {
    public:
        enum class ChipSelectState: int {
            SELECTED = 0,
            UNSELECTED = 1
        };
        using SpiInit = std::function<void(void)>;
        using SetSpiSpeed = std::function<void(const std::size_t speed)>;
        using TrancieveByte = std::function<std::uint8_t(const std::uint8_t byte)>;
        using Delay = std::function<void(const std::size_t ms)>;
        using ChipSelector = std::function<void(const ChipSelectState state)>;

        SdSpiDriver(
            const SpiInit& spi_init,
            const SetSpiSpeed& set_spi_speed,
            const TrancieveByte& trancieve_byte,
            const Delay& delay,
            const ChipSelector& chip_selector
        ): m_spi_init(spi_init), m_set_spi_speed(set_spi_speed), m_trancieve_byte(trancieve_byte), m_delay(delay), m_chip_selector(chip_selector) {
            if (!m_spi_init || !m_set_spi_speed || !m_trancieve_byte || !m_delay || !m_chip_selector) {
                throw std::invalid_argument("invalid argument(s) provided to SdSpiDriver constructor");
            }
            m_spi_init();
        }
        SdSpiDriver(const SdSpiDriver&) = default;
        SdSpiDriver& operator=(const SdSpiDriver&) = default;
        ~SdSpiDriver() noexcept;
        
        std::array<std::size_t, BlockSize> read_block(const std::size_t block_address) const;
        void write_block(const std::size_t block_address, const std::array<std::size_t, BlockSize>& data) const;
    private:
        SpiInit m_spi_init;
        SetSpiSpeed m_set_spi_speed;
        TrancieveByte m_trancieve_byte;
        Delay m_delay;
        ChipSelector m_chip_selector;
    };
}

#endif // SD_SPI_DRIVER_HPP