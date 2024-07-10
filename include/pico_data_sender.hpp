#ifndef	PICO_DATA_SENDER_HPP
#define	PICO_DATA_SENDER_HPP

#include <stdexcept>

#include "data_sender.hpp"
#include "pico_mcu_server_types.hpp"

namespace pico_mcu_server {

	class PicoDataSender: public mcu_server::DataSender<RawData> {
	public:
		enum class UartId: int {
			UART0,
			UART1
		};
		PicoDataSender(const UartId& uart_id);
		PicoDataSender(const PicoDataSender& other) = delete;
		PicoDataSender& operator=(const PicoDataSender& other) = delete;

		void send(const RawData&) const override;
	private:
		UartId m_uart_id;
	};

	PicoDataSender::PicoDataSender(const UartId& uart_id): m_uart_id(uart_id) {

	}

	inline void PicoDataSender::send(const RawData& data) const {
		throw std::runtime_error("NOT IMPLEMENTED");
	}
}

#endif // PICO_DATA_SENDER_HPP