#include <exception>
#include <vector>

#include "boards/pico.h"
#include "custom_listener.hpp"
#include "custom_receiver.hpp"
#include "custom_retriever.hpp"
#include "data.hpp"
#include "integer.hpp"
#include "json_data_parser.hpp"
#include "json_data_serializer.hpp"
#include "mcu_server.hpp"
#include "mcu_task_engine.hpp"
#include "mcu_task_type.hpp"
#include "object.hpp"
#include "pico/stdio.h"
#include "pico_data_sender.hpp"
#include "pico_gpi.hpp"
#include "pico_gpo.hpp"
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
        )
    );

    PicoUart uart;
    PicoDataSender sender(&uart, MSG_HEADER, MSG_TAIL);
    CustomReceiver receiver(MSG_HEADER, MSG_TAIL);

    McuServer<GpioId, RawData, FoodData> server(
        &task_engine,
        &sender,
        &receiver,
        JsonDataParser(),
        JsonDataSerializer()
    );
    uart.set_listener(
        CustomListener<RawData>(
            [&server](const RawData& data)-> void {
                server.feed(data);
            }
        )
    );
    uart.send("MCU SERVER STARTED\n\r");

    while (true) {
        ;
    }
    return 0;
}