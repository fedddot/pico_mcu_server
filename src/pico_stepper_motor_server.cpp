#include <exception>
#include <stdexcept>

#include "custom_creator.hpp"
#include "default_stepper_motor_data_parser.hpp"
#include "gpo.hpp"
#include "integer.hpp"
#include "inventory.hpp"
#include "json_data_parser.hpp"
#include "json_data_serializer.hpp"
#include "mcu_server.hpp"
#include "object.hpp"
#include "pico/stdio.h"
#include "pico/types.h"
#include "pico_delay.hpp"
#include "pico_gpo.hpp"
#include "pico_ipc_connection.hpp"
#include "stepper_motor.hpp"
#include "stepper_motor_tasks_factory.hpp"
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
using StepperId = int;
using TaskFactory = mcu_factory::StepperMotorTasksFactory<StepperId, GpioId>;
using MotorInventory = Inventory<StepperId, StepperMotor<GpioId>>;

static PicoIpcConnection::Baud cast_baud(uint baud);

int main(void) {
    stdio_init_all();

    PicoIpcConnection connection(
        cast_baud(PICO_IPC_BAUD),
        MSG_HEADER,
        MSG_TAIL,
        PICO_IPC_MAX_BUFF_SIZE
    );

    MotorInventory inventory;

    TaskFactory factory(
        &inventory,
        DefaultStepperMotorDataParser(),
        CustomCreator<Gpo *(const GpioId&)>(
            [](const GpioId& id)-> Gpo * {
                return new PicoGpo(id);
            }
        ),
        PicoDelay(),
        CustomCreator<Data *(int)>(
            [](int result) {
                Object report;
                report.add("result", Integer(result));
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