#ifndef	PICO_GPI_HPP
#define	PICO_GPI_HPP

#include <stdexcept>
#include <stdio.h>
#include "gpio.hpp"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "gpi.hpp"

namespace pico_mcu_server {

	class PicoGpi: public mcu_task_engine::Gpi {
	public:
		PicoGpi(int id);
		PicoGpi(const PicoGpi& other) = default;
		PicoGpi& operator=(const PicoGpi& other) = delete;
		State state() const override;
		mcu_task_engine::Gpio *clone() const override;
	private:
		int m_id;
	};

	inline PicoGpi::PicoGpi(int id): m_id(id) {
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

	inline mcu_task_engine::Gpio *PicoGpi::clone() const {
		return new PicoGpi(*this);
	}
}

#endif // PICO_GPI_HPP