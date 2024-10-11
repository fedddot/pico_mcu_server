#include <stdexcept>

#include <pico/stdio.h>
#include <string>

#include "data.hpp"
#include "gpio_manager.hpp"
#include "integer.hpp"
#include "json_request_parser.hpp"
#include "json_response_serializer.hpp"
#include "object.hpp"
#include "pico_gpi.hpp"
#include "pico_gpo.hpp"
#include "pico_ipc_connection.hpp"
#include "pico_stepper_motor.hpp"
#include "server.hpp"
#include "stepper_motor_manager.hpp"
#include "string.hpp"
#include "vendor.hpp"
#include "gpio.hpp"
#include "stepper_motor.hpp"

#ifndef MSG_HEAD
#   define MSG_HEAD "MSG_HEAD"
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
using namespace server;
using namespace server_utl;
using namespace vendor;
using namespace manager;
using namespace pico_mcu_platform;

static PicoIpcConnection::Baud cast_baud(uint baud);

static bool match(const RawData& data, const RawData& head, const RawData& tail);
static Request extract(RawData *data, const RawData& head, const RawData& tail);
static RawData serialize(const server::Response& response, const RawData& head, const RawData& tail);

static Gpio *create_gpio(const Data& data);
static StepperMotor *create_stepper_motor(const Data& data);

int main(void) {
    stdio_init_all();

    PicoIpcConnection connection(
        cast_baud(PICO_IPC_BAUD),
        [](const server::Response& response) {
            return serialize(response, MSG_HEAD, MSG_TAIL);
        },
        [](const RawData& data) {
            return match(data, MSG_HEAD, MSG_TAIL);
        },
        [](RawData *data) {
            return extract(data, MSG_HEAD, MSG_TAIL);
        }
    );

    Vendor vendor;
    vendor.register_resource(
        "gpios",
        GpioManager(create_gpio)
    );
    vendor.register_resource(
        "steppers",
        StepperMotorManager(create_stepper_motor)
    );

    Server<std::string> server(
        &connection,
        "cnc_server",
        vendor
    );

    server.run();

    while (true) {
        ;
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

inline bool match(const RawData& data, const RawData& head, const RawData& tail) {
    auto head_pos = data.find(head);
    if (RawData::npos == head_pos) {
        return false;
    }
    auto tail_pos = data.find(tail, head_pos + tail.size());
    if (RawData::npos == tail_pos) {
        return false;
    }
    return true;
}

inline Request extract(RawData *data, const RawData& head, const RawData& tail) {
    auto head_pos = data->find(head);
    if (RawData::npos == head_pos) {
        throw std::invalid_argument("missing head");
    }
    auto tail_pos = data->find(tail, head_pos + head.size());
    if (RawData::npos == tail_pos) {
        throw std::invalid_argument("missing tail");
    }
    RawData extracted(data->begin() + head_pos + head.size(), data->begin() + tail_pos);
    data->erase(data->begin() + head_pos, data->begin() + tail_pos + tail.size());
    return JsonRequestParser()(extracted);
}

inline RawData serialize(const server::Response& response, const RawData& head, const RawData& tail) {
    return head + JsonResponseSerializer()(response) + tail;
}

inline Gpio *create_gpio(const Data& data) {
    auto id = Data::cast<String>(Data::cast<Object>(data).access("id")).get();
    auto dir = static_cast<Gpio::Direction>(Data::cast<Integer>(Data::cast<Object>(data).access("dir")).get());
    switch (dir) {
    case Gpio::Direction::IN:
        return new PicoGpi(std::stoi(id));
    case Gpio::Direction::OUT:
        return new PicoGpo(std::stoi(id));
    default:
        throw std::invalid_argument("unsupported direction received");
    }
}

inline StepperMotor *create_stepper_motor(const Data& data) {
    auto ll = static_cast<unsigned int>(Data::cast<Integer>(Data::cast<Object>(data).access("left_low")).get());
    auto lh = static_cast<unsigned int>(Data::cast<Integer>(Data::cast<Object>(data).access("left_high")).get());
    auto rl = static_cast<unsigned int>(Data::cast<Integer>(Data::cast<Object>(data).access("right_low")).get());
    auto rh = static_cast<unsigned int>(Data::cast<Integer>(Data::cast<Object>(data).access("right_high")).get());
    return new PicoStepperMotor(
        PicoStepperMotor::ShouldersMapping(
            {
                {PicoStepperMotor::Shoulder::LEFT_LOW, ll},
                {PicoStepperMotor::Shoulder::LEFT_HIGH, lh},
                {PicoStepperMotor::Shoulder::RIGHT_LOW, rl},
                {PicoStepperMotor::Shoulder::RIGHT_HIGH, rh},
            }
        )
    );
}