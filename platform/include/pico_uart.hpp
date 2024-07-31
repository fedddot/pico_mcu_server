#ifndef	PICO_UART_HPP
#define	PICO_UART_HPP

#include <stdexcept>
#include <memory>

#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include "listener.hpp"
#include "pico_mcu_server_types.hpp"

namespace pico_mcu_server {

	class PicoUart {
	public:
		PicoUart();
		PicoUart(const PicoUart& other) = delete;
		PicoUart& operator=(const PicoUart& other) = delete;
		virtual ~PicoUart() noexcept;

		void send(const RawData&) const;
		void set_listener(const mcu_server::Listener<RawData>& listener);
	private:
		enum : uint {
			UART0_TX_PIN = 0,
			UART0_RX_PIN = 1,
			BAUD_RATE = 9600,
			DATA_BITS = 8,
			STOP_BITS = 1
		};
		static uart_inst_t * const m_uart;
		static std::unique_ptr<mcu_server::Listener<RawData>> s_listener;
		static void register_listener(const mcu_server::Listener<RawData>& char_listener);
		static void on_received_cb();
	};

	std::unique_ptr<mcu_server::Listener<RawData>> PicoUart::s_listener(nullptr);
	uart_inst_t * const PicoUart::m_uart(uart0);

	inline PicoUart::PicoUart() {
		uart_init(m_uart, BAUD_RATE);
		gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    	gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
		uart_set_baudrate(uart0, BAUD_RATE);
		uart_set_hw_flow(uart0, false, false);
		uart_set_format(uart0, DATA_BITS, STOP_BITS, UART_PARITY_NONE);
		uart_set_fifo_enabled(m_uart, false);
	    irq_set_exclusive_handler(UART0_IRQ, &on_received_cb);
    	irq_set_enabled(UART0_IRQ, true);
    	uart_set_irq_enables(uart0, true, false);
	}

	inline PicoUart::~PicoUart() noexcept {
		irq_set_enabled(UART0_IRQ, false);
    	uart_set_irq_enables(uart0, false, false);
    	uart_deinit(m_uart);
		gpio_set_function(UART0_TX_PIN, GPIO_FUNC_NULL);
    	gpio_set_function(UART0_RX_PIN, GPIO_FUNC_NULL);
	}

	inline void PicoUart::send(const RawData& data) const {
		for (auto ch: data) {
			uart_putc(m_uart, ch);
		}
	}

	inline void PicoUart::set_listener(const mcu_server::Listener<RawData>& char_listener) {
		if (s_listener) {
			throw std::runtime_error("uart0 listener is already initialized");
		}
		s_listener = std::unique_ptr<mcu_server::Listener<RawData>>(char_listener.clone());
	}

	inline void PicoUart::on_received_cb() {
		if (!s_listener) {
			return;
		}
		RawData data("");
		while (uart_is_readable(m_uart)) {
			data.push_back(uart_getc(m_uart));
		}
		s_listener->on_event(data);
	}
}

#endif // PICO_UART_HPP