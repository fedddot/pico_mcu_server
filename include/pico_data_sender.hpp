#ifndef	PICO_DATA_SENDER_HPP
#define	PICO_DATA_SENDER_HPP

namespace mcu_server {
	
	template <typename Tdata>
	class DataSender {
	public:
		virtual ~DataSender() noexcept = default;
		virtual void send(const Tdata&) const = 0;
	};
}

#endif // PICO_DATA_SENDER_HPP