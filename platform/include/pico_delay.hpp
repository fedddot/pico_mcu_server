#ifndef	PICO_DELAY_HPP
#define	PICO_DELAY_HPP

#include "pico/time.h"

#include "delay.hpp"

namespace pico_mcu_server {

	class PicoDelay: public mcu_platform::Delay {
	public:
		PicoDelay() = default;
		PicoDelay(const PicoDelay& other) = default;
		PicoDelay& operator=(const PicoDelay& other) = delete;

		void delay(int delay_ms) const override;
		mcu_platform::Delay *clone() const override;
	};

	inline void PicoDelay::delay(int delay_ms) const {
		sleep_ms(delay_ms);
	}

	inline mcu_platform::Delay *PicoDelay::clone() const {
		return new PicoDelay(*this);
	}
}

#endif // PICO_DELAY_HPP