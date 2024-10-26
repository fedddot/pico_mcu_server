#ifndef	PICO_GPI_HPP
#define	PICO_GPI_HPP

#include <stdexcept>

#include "hardware/gpio.h"

#include "gpio.hpp"
#include "gpi.hpp"

namespace pico_mcu_platform {

	class PicoGpi: public manager::Gpi {
	public:
		PicoGpi(int id);
		PicoGpi(const PicoGpi& other) = default;
		PicoGpi& operator=(const PicoGpi& other) = delete;
		State state() const override;
		manager::Gpio *clone() const override;
	private:
		int m_id;
	};

	inline PicoGpi::PicoGpi(int id): m_id(id) {
		if ((0 == m_id) || (1 == m_id)) {
			throw std::invalid_argument("GPIOs id = 0, 1 are reserved for UART0");
		}
		gpio_init(m_id);
        gpio_set_dir(m_id, GPIO_IN);
		gpio_pull_up(m_id);
	}

	inline PicoGpi::State PicoGpi::state() const {
		if (gpio_get(m_id)) {
			return State::HIGH;
		}
		return State::LOW;
	}

	inline manager::Gpio *PicoGpi::clone() const {
		return new PicoGpi(*this);
	}
}

#endif // PICO_GPI_HPP