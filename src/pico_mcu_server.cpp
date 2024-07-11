#include <exception>
#include <vector>

#include "boards/pico.h"
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

    PicoDataSender sender(PicoDataSender::UartId::UART0);

    CustomReceiver receiver(MSG_HEADER, MSG_TAIL);

    McuServer<GpioId, RawData, FoodData> server(
        &task_engine,
        &sender,
        &receiver,
        JsonDataParser(),
        JsonDataSerializer()
    );

    GpioId gpio_id(PICO_DEFAULT_LED_PIN);
    Object create_task_data;
    create_task_data.add("ctor_id", Integer(static_cast<int>(McuTaskType::CREATE_GPIO)));
    create_task_data.add("gpio_id", Integer(static_cast<int>(gpio_id)));
    create_task_data.add("gpio_dir", Integer(static_cast<int>(Gpio::Direction::OUT)));

    Object set_task_data;
    set_task_data.add("ctor_id", Integer(static_cast<int>(McuTaskType::SET_GPIO)));
    set_task_data.add("gpio_id", Integer(static_cast<int>(gpio_id)));
    set_task_data.add("gpio_state", Integer(static_cast<int>(Gpio::State::HIGH)));

    const std::vector<std::string> sequence {
        std::string(MSG_HEADER) + JsonDataSerializer().serialize(create_task_data) + std::string(MSG_TAIL),
        std::string(MSG_HEADER) + JsonDataSerializer().serialize(set_task_data) + std::string(MSG_TAIL)
    };

    for (auto task_data: sequence) {
        server.feed(task_data);
    }

    return 0;
}