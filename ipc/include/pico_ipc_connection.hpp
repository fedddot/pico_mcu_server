#ifndef	PICO_IPC_CONNECTION_HPP
#define	PICO_IPC_CONNECTION_HPP

#include <functional>

#include "buffered_ipc_connection.hpp"
#include "ipc_connection.hpp"
#include "request.hpp"
#include "response.hpp"

namespace pico_mcu_ipc {

	using RawData = std::string;
	using SubscriberId = std::string;
	
	class PicoIpcConnection: public ipc::IpcConnection<SubscriberId, server::Request, server::Response> {
	public:
		using Callback = typename ipc::IpcConnection<SubscriberId, server::Request, server::Response>::Callback;
		using RequestMatcher = typename ipc_utl::BufferedIpcConnection<SubscriberId, RawData>::RequestMatcher;
		using RequestExtractor = typename ipc_utl::BufferedIpcConnection<SubscriberId, RawData>::RequestExtractor;
		using ResponseSerializer = std::function<RawData(const server::Response&)>;
		
		enum class Baud: int {
			B9600,
			B115200
		};
		PicoIpcConnection(
			const Baud& baud,
			const ResponseSerializer& response_serializer,
			const RequestMatcher& request_matcher,
			const RequestExtractor& request_extractor
		);
		PicoIpcConnection(const PicoIpcConnection&) = delete;
		PicoIpcConnection& operator=(const PicoIpcConnection&) = delete;
		~PicoIpcConnection() noexcept override;

		void subscribe(const SubscriberId& id, const Callback& cb) override;
		void unsubscribe(const SubscriberId& id) override;
		bool is_subscribed(const SubscriberId& id) const override;
		void send(const server::Response& outgoing_data) const override;
	private:
		enum : uint {
			UART0_TX_PIN = 0,
			UART0_RX_PIN = 1,
			DATA_BITS = 8,
			STOP_BITS = 1
		};
		ResponseSerializer m_response_serializer;
		using BufferedConnection = ipc_utl::BufferedIpcConnection<SubscriberId, RawData>;
		BufferedConnection m_buffered_connection;
		
		static BufferedConnection *s_buffered_connection;
		static void init_uart(const Baud& baud, BufferedConnection *buffered_connection);
		static void uninit_uart();
		static void on_received_cb();
		static void send_data(const RawData& data);
		static uint baud_to_uint(const Baud& baud);
	};

	inline PicoIpcConnection::PicoIpcConnection(
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

	inline PicoIpcConnection::~PicoIpcConnection() noexcept {
		uninit_uart();
	}

	inline void PicoIpcConnection::subscribe(const SubscriberId& id, const Callback& cb) {
		m_buffered_connection.subscribe(id, cb);
	}

	inline void PicoIpcConnection::unsubscribe(const SubscriberId& id) {
		m_buffered_connection.unsubscribe(id);
	}

	inline bool PicoIpcConnection::is_subscribed(const SubscriberId& id) const {
		return m_buffered_connection.is_subscribed(id);
	}

	inline void PicoIpcConnection::send(const server::Response& outgoing_data) const {
		m_buffered_connection.send(outgoing_data);
	}
}

#endif // PICO_IPC_CONNECTION_HPP