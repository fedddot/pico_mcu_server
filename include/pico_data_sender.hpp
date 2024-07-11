#ifndef	PICO_DATA_SENDER_HPP
#define	PICO_DATA_SENDER_HPP

#include <stdexcept>

#include "data_sender.hpp"
#include "pico_mcu_server_types.hpp"
#include "pico_uart.hpp"

namespace pico_mcu_server {

	class PicoDataSender: public mcu_server::DataSender<RawData> {
	public:
		PicoDataSender(PicoUart *uart, const RawData& head, const RawData& tail);
		PicoDataSender(const PicoDataSender& other) = delete;
		PicoDataSender& operator=(const PicoDataSender& other) = delete;

		void send(const RawData&) const override;
	private:
		PicoUart *m_uart;
		RawData m_head;
		RawData m_tail;
	};

	inline PicoDataSender::PicoDataSender(PicoUart *uart, const RawData& head, const RawData& tail): m_uart(uart), m_head(head), m_tail(tail) {
		if (!m_uart) {
			throw std::invalid_argument("invalid uart ptr received");
		}
	}

	inline void PicoDataSender::send(const RawData& data) const {
		m_uart->send(m_head + data + m_tail);
	}
}

#endif // PICO_DATA_SENDER_HPP