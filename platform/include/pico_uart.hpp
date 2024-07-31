#ifndef	PICO_UART_HPP
#define	PICO_UART_HPP

#include <stdexcept>
#include <string>

#include "buffered_message_receiver.hpp"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "message_receiver.hpp"
#include "message_sender.hpp"

namespace pico_mcu_platform {
	
	using UartData = std::string;
	
	class PicoUart: public mcu_platform::MessageReceiver<UartData>, public mcu_platform::MessageSender<UartData> {
	public:
		enum Baud {
			BAUD9600,
			BAUD115200
		};
		
		PicoUart(const UartData& header, const UartData& tail, const std::size_t& max_buffer_size, const Baud& baud);
		PicoUart(const PicoUart& other) = delete;
		PicoUart& operator=(const PicoUart& other) = delete;
		~PicoUart() noexcept override;
		bool message_received() const override;
		UartData receive() override;
		void send(const UartData&) const override;
	private:
		UartData m_header;
		UartData m_tail;
		mcu_platform_utl::BufferedReceiver m_receiver;
		enum : uint {
			UART0_TX_PIN = 0,
			UART0_RX_PIN = 1,
			DATA_BITS = 8,
			STOP_BITS = 1
		};
		static uint baud_to_uint(const Baud& baud);
		static void on_received_cb(void);
		static mcu_platform_utl::BufferedReceiver *s_receiver;
	};

	inline PicoUart::PicoUart(const UartData& header, const UartData& tail, const std::size_t& max_buffer_size, const Baud& baud): m_header(header), m_tail(tail), m_receiver(header, tail, max_buffer_size) {
		if (nullptr != s_receiver) {
			throw std::runtime_error("uart receiver has already been initialized");
		}
		uart_init(uart0, baud_to_uint(baud));
		gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    	gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
		uart_set_baudrate(uart0, baud_to_uint(baud));
		uart_set_hw_flow(uart0, false, false);
		uart_set_format(uart0, DATA_BITS, STOP_BITS, UART_PARITY_NONE);
		uart_set_fifo_enabled(uart0, false);
	    irq_set_exclusive_handler(UART0_IRQ, &on_received_cb);
    	irq_set_enabled(UART0_IRQ, true);
    	uart_set_irq_enables(uart0, true, false);
		s_receiver = &m_receiver;
	}

	inline PicoUart::~PicoUart() noexcept {
		irq_set_enabled(UART0_IRQ, false);
    	uart_set_irq_enables(uart0, false, false);
    	uart_deinit(uart0);
		gpio_set_function(UART0_TX_PIN, GPIO_FUNC_NULL);
    	gpio_set_function(UART0_RX_PIN, GPIO_FUNC_NULL);
		s_receiver = nullptr;
	}

	inline bool PicoUart::message_received() const {
		return m_receiver.message_received();
	}

	inline UartData PicoUart::receive() {
		return m_receiver.receive();
	}

	inline void PicoUart::send(const UartData& data) const {
		for (auto ch: m_header + data + m_tail) {
			uart_putc(uart0, ch);
		}
	}

	inline void PicoUart::on_received_cb(void) {
		if (!s_receiver) {
			return;
		}
		UartData data("");
		while (uart_is_readable(uart0)) {
			data.push_back(uart_getc(uart0));
		}
		s_receiver->feed(data);
	}
}

#endif // PICO_UART_HPP