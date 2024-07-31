#ifndef	PICO_DELAY_HPP
#define	PICO_DELAY_HPP

#include <memory>

#include "pico/stdlib.h"

#include "delay.hpp"

namespace pico_mcu_server {

	class PicoDelay: public mcu_task_engine::Delay {
	public:
		PicoDelay(int delay_ms);
		PicoDelay(const PicoDelay& other) = delete;
		PicoDelay& operator=(const PicoDelay& other) = delete;

		void delay() const override;
	private:
		int m_delay_ms;
	};

	inline PicoDelay::PicoDelay(int delay_ms): m_delay_ms(delay_ms) {

	}

	inline void PicoDelay::delay() const {
		sleep_ms(m_delay_ms);
	}
}

#endif // PICO_DELAY_HPP