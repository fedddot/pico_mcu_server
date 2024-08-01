#include <stdexcept>

#include "pico_uart.hpp"

using namespace pico_mcu_platform;

int main(void) {
	PicoUart uart("abc", "efg", 100, PicoUart::Baud::BAUD9600);

	if (uart.message_received()) {
		return -1;
	}

	uart.send("hahaha"); // TX is closed on RX
	if (!uart.message_received()) {
		return -1;
	}
	if ("hahaha" != uart.receive()) {
		return -1;
	}

	return 0;
}
