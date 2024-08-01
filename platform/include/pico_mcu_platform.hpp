#ifndef	PICO_MCU_PLATFORM_HPP
#define	PICO_MCU_PLATFORM_HPP

#include "delay.hpp"
#include "gpio.hpp"
#include "inventory.hpp"

#include "pico_delay.hpp"
#include "pico_gpi.hpp"
#include "pico_gpo.hpp"
#include "platform.hpp"

namespace pico_mcu_platform {

	class PicoMcuPlatform: public mcu_platform::Platform<int> {
	public:
		PicoMcuPlatform();
		PicoMcuPlatform(const PicoMcuPlatform& other) = delete;
		PicoMcuPlatform& operator=(const PicoMcuPlatform& other) = delete;

		mcu_platform::Delay *create_delay() const override;
		mcu_platform::Gpio *create_gpio(const int& id, const mcu_platform::Gpio::Direction& dir) const override;
		mcu_platform::Inventory<int, mcu_platform::Gpio> *gpio_inventory() const override;
	private:
		mutable mcu_platform::Inventory<int, mcu_platform::Gpio> m_gpio_inventory;
	};

	inline mcu_platform::Delay *PicoMcuPlatform::create_delay() const {
		return new PicoDelay();
	}
	
	inline mcu_platform::Gpio *PicoMcuPlatform::create_gpio(const int& id, const mcu_platform::Gpio::Direction& dir) const {
		switch (dir) {
		case mcu_platform::Gpio::Direction::IN:
			return new PicoGpi(id);
		case mcu_platform::Gpio::Direction::OUT:
			return new PicoGpo(id);
		default:
			throw std::invalid_argument("unsupported GPIO direction received");
		}
	}
	
	inline mcu_platform::Inventory<int, mcu_platform::Gpio> *PicoMcuPlatform::gpio_inventory() const {
		return &m_gpio_inventory;
	}
}

#endif // PICO_MCU_PLATFORM_HPP