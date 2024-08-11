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
	template <typename Tgpio_id, typename Ttask_id>
	class PicoMcuPlatform: public mcu_platform::Platform<Tgpio_id, Ttask_id> {
	public:
		using PersistentTask = typename mcu_platform::Platform<Tgpio_id, Ttask_id>::PersistentTask;
		using GpioInventory = mcu_platform::Inventory<Tgpio_id, mcu_platform::Gpio>;
		using TaskInventory = mcu_platform::Inventory<Ttask_id, PersistentTask>;

		PicoMcuPlatform() = default;
		PicoMcuPlatform(const PicoMcuPlatform& other) = delete;
		PicoMcuPlatform& operator=(const PicoMcuPlatform& other) = delete;

		mcu_platform::Delay *create_delay() const override;
		mcu_platform::Gpio *create_gpio(const int& id, const mcu_platform::Gpio::Direction& dir) const override;
		GpioInventory *gpio_inventory() const override;
		TaskInventory *task_inventory() const override;
	private:
		mutable GpioInventory m_gpio_inventory;
		mutable TaskInventory m_task_inventory;
	};

	template <typename Tgpio_id, typename Ttask_id>
	inline mcu_platform::Delay *PicoMcuPlatform<Tgpio_id, Ttask_id>::create_delay() const {
		return new PicoDelay();
	}
	
	template <typename Tgpio_id, typename Ttask_id>
	inline mcu_platform::Gpio *PicoMcuPlatform<Tgpio_id, Ttask_id>::create_gpio(const int& id, const mcu_platform::Gpio::Direction& dir) const {
		switch (dir) {
		case mcu_platform::Gpio::Direction::IN:
			return new PicoGpi(static_cast<int>(id));
		case mcu_platform::Gpio::Direction::OUT:
			return new PicoGpo(static_cast<int>(id));
		default:
			throw std::invalid_argument("unsupported GPIO direction received");
		}
	}
	
	template <typename Tgpio_id, typename Ttask_id>
	inline typename PicoMcuPlatform<Tgpio_id, Ttask_id>::GpioInventory *PicoMcuPlatform<Tgpio_id, Ttask_id>::gpio_inventory() const {
		return &m_gpio_inventory;
	}

	template <typename Tgpio_id, typename Ttask_id>
	inline typename PicoMcuPlatform<Tgpio_id, Ttask_id>::TaskInventory *PicoMcuPlatform<Tgpio_id, Ttask_id>::task_inventory() const {
		return &m_task_inventory;
	}
}

#endif // PICO_MCU_PLATFORM_HPP