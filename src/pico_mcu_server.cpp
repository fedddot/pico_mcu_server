#include <exception>
#include <stdexcept>

#include "array.hpp"
#include "custom_creator.hpp"
#include "custom_parser.hpp"
#include "data.hpp"
#include "gpio.hpp"
#include "integer.hpp"
#include "json_data_parser.hpp"
#include "json_data_serializer.hpp"
#include "mcu_factory.hpp"
#include "mcu_server.hpp"
#include "object.hpp"
#include "pico/stdio.h"
#include "pico/types.h"
#include "pico_ipc_connection.hpp"
#include "pico_mcu_platform.hpp"
#include "string.hpp"

#ifndef MSG_HEADER
#   define MSG_HEADER "MSG_HEADER"
#endif

#ifndef MSG_TAIL
#   define MSG_TAIL "MSG_TAIL"
#endif

#ifndef PICO_IPC_BAUD
#   define PICO_IPC_BAUD 9600UL
#endif

#ifndef PICO_IPC_MAX_BUFF_SIZE
#   define PICO_IPC_MAX_BUFF_SIZE 1000UL
#endif

using namespace pico_mcu_ipc;
using namespace mcu_factory;
using namespace pico_mcu_platform;
using namespace mcu_server;
using namespace mcu_platform;
using namespace mcu_server_utl;

using GpioId = int;
using TaskFactory = mcu_factory::McuFactory<GpioId>;
using TaskType = typename TaskFactory::TaskType;

static PicoIpcConnection::Baud cast_baud(uint baud);

int main(void) {
    stdio_init_all();

    PicoIpcConnection connection(
        cast_baud(PICO_IPC_BAUD),
        MSG_HEADER,
        MSG_TAIL,
        PICO_IPC_MAX_BUFF_SIZE
    );

    pico_mcu_platform::PicoMcuPlatform platform;

    mcu_factory::McuFactory<GpioId> factory(
        &platform,
        CustomParser<TaskType(const Data&)>(
            [](const Data& data) {
                return static_cast<TaskType>(Data::cast<Integer>(Data::cast<Object>(data).access("task_type")).get());
            }
        ),
        CustomParser<GpioId(const Data&)>(
            [](const Data& data) {
                return static_cast<GpioId>(Data::cast<Integer>(Data::cast<Object>(data).access("gpio_id")).get());
            }
        ),
        CustomParser<Gpio::Direction(const Data&)>(
            [](const Data& data) {
                return static_cast<Gpio::Direction>(Data::cast<Integer>(Data::cast<Object>(data).access("gpio_dir")).get());
            }
        ),
        CustomParser<Gpio::State(const Data&)>(
            [](const Data& data) {
                return static_cast<Gpio::State>(Data::cast<Integer>(Data::cast<Object>(data).access("gpio_state")).get());
            }
        ),
        CustomParser<Array(const Data&)>(
            [](const Data& data) {
                return Data::cast<Array>(Data::cast<Object>(data).access("tasks"));
            }
        ),
        CustomParser<unsigned int(const Data&)>(
            [](const Data& data) {
                return static_cast<unsigned int>(Data::cast<Integer>(Data::cast<Object>(data).access("delay_ms")).get());
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
        CustomCreator<Data *(const Array& tasks_results)>(
            [](const Array& tasks_results) {
                Object report;
                report.add("result", Integer(0));
                report.add("reports", tasks_results);
                return report.clone();
            }
        )
    );

    McuServer<PicoIpcData> server(
        JsonDataParser(),
        JsonDataSerializer(),
        factory,
        CustomCreator<Data *(const std::exception& e)>(
            [](const std::exception& e) {
                Object report;
                report.add("result", Integer(-1));
                report.add("what", String(e.what()));
                return report.clone();
            }
        )
    );

    while (true) {
        try {
            if (!connection.readable()) {
                continue;
            }
            auto incoming_msg = connection.read();
            auto report = server.run(incoming_msg);
            connection.send(report);
        } catch (const std::exception& e) {
            Object fatal_failure_report;
            fatal_failure_report.add("msg", String("an exception catched in server main loop"));
            fatal_failure_report.add("what", String(e.what()));
            connection.send(JsonDataSerializer().serialize(fatal_failure_report));
        }
    }
    return 0;
}

inline PicoIpcConnection::Baud cast_baud(uint baud) {
    switch (baud) {
    case 9600UL:
        return PicoIpcConnection::Baud::B9600;
    case 115200UL:
        return PicoIpcConnection::Baud::B115200;
    default:
        throw std::invalid_argument("unsupported baud received");
    }
}