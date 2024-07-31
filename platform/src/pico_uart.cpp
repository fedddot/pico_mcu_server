#include "buffered_message_receiver.hpp"

#include "pico_uart.hpp"

mcu_platform_utl::BufferedReceiver *pico_mcu_platform::PicoUart::s_receiver(nullptr);