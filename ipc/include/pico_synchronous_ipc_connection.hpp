#ifndef	PICO_SYNCHRONOUS_IPC_CONNECTION_HPP
#define	PICO_SYNCHRONOUS_IPC_CONNECTION_HPP

#include <functional>

#include "buffered_ipc_connection.hpp"
#include "ipc_connection.hpp"
#include "request.hpp"
#include "response.hpp"

namespace pico_mcu_ipc {

	using RawData = std::string;
	using SubscriberId = std::string;
	
	class PicoSynchronousIpcConnection: public ipc::IpcConnection<SubscriberId, server::Request, server::Response> {
	public:
		using Callback = typename ipc::IpcConnection<SubscriberId, server::Request, server::Response>::Callback;
		using RequestMatcher = typename ipc::BufferedIpcConnection<SubscriberId, RawData>::RequestMatcher;
		using RequestExtractor = typename ipc::BufferedIpcConnection<SubscriberId, RawData>::RequestExtractor;
		using ResponseSerializer = std::function<RawData(const server::Response&)>;
		
		enum class Baud: int {
			B9600,
			B115200
		};
		PicoSynchronousIpcConnection(
			const Baud& baud,
			const ResponseSerializer& response_serializer,
			const RequestMatcher& request_matcher,
			const RequestExtractor& request_extractor
		);
		PicoSynchronousIpcConnection(const PicoSynchronousIpcConnection&) = delete;
		PicoSynchronousIpcConnection& operator=(const PicoSynchronousIpcConnection&) = delete;
		~PicoSynchronousIpcConnection() noexcept override;

		void subscribe(const SubscriberId& id, const Callback& cb) override;
		void unsubscribe(const SubscriberId& id) override;
		bool is_subscribed(const SubscriberId& id) const override;
		void send(const server::Response& outgoing_data) const override;
		void loop();
	private:
		enum : uint {
			UART0_TX_PIN = 0,
			UART0_RX_PIN = 1,
			DATA_BITS = 8,
			STOP_BITS = 1
		};
		ResponseSerializer m_response_serializer;
		using BufferedConnection = ipc::BufferedIpcConnection<SubscriberId, RawData>;
		BufferedConnection m_buffered_connection;
		
		static void init_uart(const Baud& baud, BufferedConnection *buffered_connection);
		static void uninit_uart();
		static void send_data(const RawData& data);
		static uint baud_to_uint(const Baud& baud);
	};

	inline PicoSynchronousIpcConnection::PicoSynchronousIpcConnection(
		const Baud& baud,
		const ResponseSerializer& response_serializer,
		const RequestMatcher& request_matcher,
		const RequestExtractor& request_extractor
	): m_response_serializer(response_serializer), m_buffered_connection(
		request_matcher,
		request_extractor,
		[this](const server::Response& response) {
			send_data(m_response_serializer(response));
		}
	) {
		init_uart(baud, &m_buffered_connection);
	}

	inline PicoSynchronousIpcConnection::~PicoSynchronousIpcConnection() noexcept {
		uninit_uart();
	}

	inline void PicoSynchronousIpcConnection::subscribe(const SubscriberId& id, const Callback& cb) {
		m_buffered_connection.subscribe(id, cb);
	}

	inline void PicoSynchronousIpcConnection::unsubscribe(const SubscriberId& id) {
		m_buffered_connection.unsubscribe(id);
	}

	inline bool PicoSynchronousIpcConnection::is_subscribed(const SubscriberId& id) const {
		return m_buffered_connection.is_subscribed(id);
	}

	inline void PicoSynchronousIpcConnection::send(const server::Response& outgoing_data) const {
		m_buffered_connection.send(outgoing_data);
	}
}

#endif // PICO_SYNCHRONOUS_IPC_CONNECTION_HPP