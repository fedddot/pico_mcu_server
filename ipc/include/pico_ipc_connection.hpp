#ifndef	PICO_IPC_CONNECTION_HPP
#define	PICO_IPC_CONNECTION_HPP

#include "buffered_custom_ipc_connection.hpp"
#include "hardware/uart.h"
#include "ipc_connection.hpp"
#include <stdexcept>
#include <string>

namespace pico_mcu_ipc {

	using PicoIpcData = std::string;
	
	class PicoIpcConnection: public mcu_ipc::IpcConnection<PicoIpcData> {
	public:
		enum class Baud: int {
			B9600,
			B115200
		};
		PicoIpcConnection(const Baud& baud, const PicoIpcData& head, const PicoIpcData& tail, const std::size_t& max_buff_size);
		PicoIpcConnection(const PicoIpcConnection&) = delete;
		PicoIpcConnection& operator=(const PicoIpcConnection&) = delete;
		~PicoIpcConnection() noexcept override;

		bool readable() const override;
		PicoIpcData read() override;
		void send(const PicoIpcData& data) const override;
	private:
		enum : uint {
			UART0_TX_PIN = 0,
			UART0_RX_PIN = 1,
			DATA_BITS = 8,
			STOP_BITS = 1
		};
		using CustomConnection = mcu_ipc_utl::BufferedCustomIpcConnection<PicoIpcData>;
		
		CustomConnection m_connection;

		static CustomConnection *s_connection;
		static void on_received_cb();
		static void send_data(const PicoIpcData& data);
	};

	inline PicoIpcConnection::PicoIpcConnection(const Baud& baud, const PicoIpcData& head, const PicoIpcData& tail, const std::size_t& max_buff_size): m_connection(head, tail, max_buff_size, send_data) {
		if (nullptr != s_connection) {
			throw std::runtime_error("uart0 connection is already created");
		}
		s_connection = &m_connection;
	}

	inline PicoIpcConnection::~PicoIpcConnection() noexcept {
		s_connection = nullptr;
	}

	inline bool PicoIpcConnection::readable() const {
		return m_connection.readable();
	}

	inline PicoIpcData PicoIpcConnection::read() {
		return m_connection.read();
	}

	inline void PicoIpcConnection::send(const std::string& data) const {
		m_connection.send(data);
	}

	inline void PicoIpcConnection::on_received_cb() {
		PicoIpcData data("");
		while (uart_is_readable(uart0)) {
			data.push_back(uart_getc(uart0));
		}
		if (!s_connection) {
			return;
		}
		s_connection->feed(data);
	}

	inline void PicoIpcConnection::send_data(const PicoIpcData& data) {
		for (auto ch: data) {
			uart_putc(uart0, ch);
		}
	}
}

#endif // PICO_IPC_CONNECTION_HPP