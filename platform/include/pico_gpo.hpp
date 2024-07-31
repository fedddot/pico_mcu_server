#ifndef	PICO_GPO_HPP
#define	PICO_GPO_HPP

#include <stdexcept>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "gpo.hpp"

namespace pico_mcu_server {

	class PicoGpo: public mcu_task_engine::Gpo {
	public:
		PicoGpo(int id);
		PicoGpo(const PicoGpo& other) = default;
		PicoGpo& operator=(const PicoGpo& other) = delete;
		State state() const override;
		void set_state(const State& state) override;
		mcu_task_engine::Gpio *clone() const override;
	private:
		uint m_id;
	};

	inline PicoGpo::PicoGpo(int id): m_id(static_cast<uint>(id)) {
		gpio_init(m_id);
        gpio_set_dir(m_id, GPIO_OUT);
	}

	inline PicoGpo::State PicoGpo::state() const {
		return gpio_get(m_id) ? State::HIGH : State::LOW;
	}

	inline void PicoGpo::set_state(const State& state) {
		gpio_put(m_id, (State::HIGH == state) ? 1 : 0);
	}

	inline mcu_task_engine::Gpio *PicoGpo::clone() const {
		return new PicoGpo(*this);
	}
}

#endif // PICO_GPO_HPP