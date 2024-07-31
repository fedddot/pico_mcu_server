#include <stdexcept>

#include "pico_uart.hpp"

using namespace pico_mcu_platform;

int main(void) {
	PicoUart uart("abc", "efg", 100, PicoUart::Baud::BAUD9600);

	return 0;
}
