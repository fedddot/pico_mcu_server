#ifndef	PICO_IPC_CONNECTION_HPP
#define	PICO_IPC_CONNECTION_HPP

#include "ipc_connection.hpp"
#include <string>

namespace pico_mcu_ipc {

	class PicoIpcConnection: mcu_ipc::IpcConnection<std::string> {
	public:
		bool readable() const override;
		std::string read() override;
		void send(const std::string& data) const override;
	};
}

#endif // PICO_IPC_CONNECTION_HPP