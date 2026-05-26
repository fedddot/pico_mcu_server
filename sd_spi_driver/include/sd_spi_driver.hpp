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

        enum class SdCommand: std::uint8_t {
            CMD0 = 0x40 + 0,
            CMD1 = 0x40 + 1,
            CMD8 = 0x40 + 8,
            CMD9 = 0x40 + 9,
            CMD16 = 0x40 + 16,
            CMD17 = 0x40 + 17,
            CMD24 = 0x40 + 24,
            CMD42 = 0x40 + 42,
            CMD55 = 0x40 + 55,
            CMD58 = 0x40 + 58,
            ACMD41 = 0x40 + 41
        };

        static std::uint8_t calculate_crc(const SdCommand cmd, std::uint32_t arg) {
            (void)arg;
            enum: std::uint8_t {
                DUMMY_CRC = 0x01,
                CMD0_CRC = 0x95,
                CMD8_CRC = 0x87
            };
            switch (cmd) {
            case SdCommand::CMD0:
                return CMD0_CRC;
            case SdCommand::CMD8:
                return CMD8_CRC;
            default:
                return DUMMY_CRC;
            }
        }

        std::uint8_t send_command(const SdCommand cmd, std::uint32_t arg) {
            m_chip_selector(ChipSelectState::SELECTED);
            m_trancieve_byte(cmd);
            m_trancieve_byte((std::uint8_t)(arg >> 24));
            m_trancieve_byte((std::uint8_t)(arg >> 16));
            m_trancieve_byte((std::uint8_t)(arg >> 8 ));
            m_trancieve_byte((std::uint8_t)(arg >> 0 ));
            m_trancieve_byte(calculate_crc(cmd, arg));

            enum: std::size_t { ATTEMPTS_NUMBER = 50UL };
            auto attempts = ATTEMPTS_NUMBER;
            std::uint8_t res(0xFF);
            while (attempts) {
                res = m_trancieve_byte(0xFF);
                if (res != 0xFF) {
                    break;
                }
                --attempts;
            }
            m_chip_selector(ChipSelectState::UNSELECTED);
            return res;
        }
    };
}

#endif // SD_SPI_DRIVER_HPP