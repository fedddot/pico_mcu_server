#include <exception>
#include <vector>
#include <map>
#include <string>

#include "boards/pico.h"
#include "custom_listener.hpp"
#include "custom_receiver.hpp"
#include "custom_retriever.hpp"
#include "data.hpp"
#include "hardware/clocks.h"
#include "integer.hpp"
#include "decoding_data_parser.hpp"
#include "encoding_data_serializer.hpp"
#include "mcu_server.hpp"
#include "mcu_task_engine.hpp"
#include "mcu_task_type.hpp"
#include "object.hpp"
#include "pico/stdio.h"
#include "pico_data_sender.hpp"
#include "pico_gpi.hpp"
#include "pico_gpo.hpp"
#include "pico_delay.hpp"
#include "pico_mcu_server_types.hpp"
#include "pico_uart.hpp"
#include "string.hpp"

using namespace mcu_server_utl;
using namespace pico_mcu_server;
using namespace mcu_task_engine;
using namespace mcu_task_engine_utl;
using namespace engine;
using namespace engine_utl;
using namespace mcu_server;

#ifndef MSG_HEADER
#   define MSG_HEADER "MSG_HEADER"
#endif

#ifndef MSG_TAIL
#   define MSG_TAIL "MSG_TAIL"
#endif

int main(void) {
    stdio_init_all();
    // clocks_init();
    McuTaskEngine<GpioId> task_engine(
        CustomRetriever<McuTaskType(const Data&)>(
            [](const Data& data) {
                return static_cast<McuTaskType>(Data::cast<Integer>(Data::cast<Object>(data).access("ctor_id")).get());
            }
        ),
        CustomCreator<Data *(const std::exception&)>(
            [](const std::exception& e) {
                Object report;
                report.add("result", Integer(-1));
                report.add("what", String(std::string(e.what())));
                return report.clone();
            }
        ),
        CustomRetriever<GpioId(const Data&)>(
            [](const Data& data) {
                return static_cast<GpioId>(Data::cast<Integer>(Data::cast<Object>(data).access("gpio_id")).get());
            }
        ),
        CustomRetriever<Gpio::Direction(const Data&)>(
            [](const Data& data) {
                return static_cast<Gpio::Direction>(Data::cast<Integer>(Data::cast<Object>(data).access("gpio_dir")).get());
            }
        ),
        CustomRetriever<Gpio::State(const Data&)>(
            [](const Data& data) {
                return static_cast<Gpio::State>(Data::cast<Integer>(Data::cast<Object>(data).access("gpio_state")).get());
            }
        ),
        CustomRetriever<Data *(const Data&)>(
            [](const Data& data) {
                return Data::cast<Object>(data).access("tasks").clone();
            }
        ),
        CustomRetriever<int(const Data&)>(
            [](const Data& data) {
                return Data::cast<Integer>(Data::cast<Object>(data).access("delay_ms")).get();
            }
        ),
        CustomCreator<Gpio *(const GpioId&, const Gpio::Direction&)>(
            [](const GpioId& id, const Gpio::Direction& dir)-> Gpio * {
                switch (dir) {
                case Gpio::Direction::IN:
                    return new PicoGpi(id);
                case Gpio::Direction::OUT:
                    return new PicoGpo(id);
                default:
                    throw std::invalid_argument("unsupported GPIO direction");
                }
            }
        ),
        CustomCreator<Data *(int)>(
            [](int result) {
                Object report;
                report.add("result", Integer(result));
                return report.clone();
            }
        ),
        CustomCreator<Data *(int, const Gpio::State&)>(
            [](int result, const Gpio::State& state) {
                Object report;
                report.add("result", Integer(result));
                report.add("gpio_state", Integer(static_cast<int>(state)));
                return report.clone();
            }
        ),
        CustomCreator<Delay *(int)>(
            [](int delay_ms) {
                return new PicoDelay(delay_ms);
            }
        )
    );

    PicoUart uart;
    PicoDataSender sender(&uart, MSG_HEADER, MSG_TAIL);
    CustomReceiver receiver(MSG_HEADER, MSG_TAIL);

    const std::map<std::string, std::string> conversion_map {
        {"ctor_id", "0"},
        {"gpio_id", "1"},
        {"gpio_dir", "2"},
        {"gpio_state", "3"},
        {"delay_ms", "4"},
        {"tasks", "5"},
        {"result", "6"},
        {"reports", "7"},
        {"what", "8"}
    };

    McuServer<GpioId, RawData, FoodData> server(
        &task_engine,
        &sender,
        &receiver,
        DecodingDataParser(conversion_map),
        EncodingDataSerializer(conversion_map)
    );
    RawData incoming_data("");
    uart.set_listener(
        CustomListener<RawData>(
            [&incoming_data](const RawData& data)-> void {
                incoming_data.insert(incoming_data.end(), data.begin(), data.end());
            }
        )
    );
    uart.send("MCU SERVER STARTED\n\r");

    while (true) {
        if (incoming_data.empty()) {
            continue;
        }
        server.feed(incoming_data);
        incoming_data = "";
    }
    return 0;
}