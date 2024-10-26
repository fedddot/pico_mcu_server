#include <map>
#include <stdexcept>
#include <string>

#include "pico/stdio.h"
#include "pico/time.h"

#include "cnc_server.hpp"
#include "data.hpp"
#include "gpio.hpp"
#include "integer.hpp"
#include "json_request_parser.hpp"
#include "json_response_serializer.hpp"
#include "linear_movement.hpp"
#include "object.hpp"
#include "pico_gpi.hpp"
#include "pico_gpo.hpp"
#include "pico_stepper_motor.hpp"
#include "pico_synchronous_ipc_connection.hpp"
#include "request.hpp"
#include "server_exception.hpp"
#include "server_types.hpp"
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
using namespace cnc_server;
using namespace server_utl;
using namespace vendor;
using namespace manager;
using namespace pico_mcu_platform;

static PicoSynchronousIpcConnection::Baud cast_baud(uint baud);

static bool match(const RawData& data, const RawData& head, const RawData& tail);
static Request extract(RawData *data, const RawData& head, const RawData& tail);
static RawData serialize(const server::Response& response, const RawData& head, const RawData& tail);

static Gpio *create_gpio(const Data& create_body);
static StepperMotor *create_stepper_motor(const Data& create_body);
static void timeout(const LinearMovement::TimeUnit& timeout_us);

int main(void) {
    stdio_init_all();

    PicoSynchronousIpcConnection connection(
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

    CncServer<std::string> server(
        &connection,
        "cnc_server",
        create_gpio,
        create_stepper_motor,
        timeout
    );

    server.run();

    while (true) {
        connection.loop();
    }
    return 0;
}

inline PicoSynchronousIpcConnection::Baud cast_baud(uint baud) {
    switch (baud) {
    case 9600UL:
        return PicoSynchronousIpcConnection::Baud::B9600;
    case 115200UL:
        return PicoSynchronousIpcConnection::Baud::B115200;
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

inline Gpio *create_gpio(const Data& create_cfg) {
    const auto& cfg_obj(Data::cast<Object>(create_cfg));
    const auto id(Data::cast<Integer>(cfg_obj.access("gpio_id")).get());
    auto dir = static_cast<Gpio::Direction>(Data::cast<Integer>(cfg_obj.access("dir")).get());
    switch (dir) {
    case Gpio::Direction::IN:
        return new PicoGpi(id);
    case Gpio::Direction::OUT:
        return new PicoGpo(id);
    default:
        throw ServerException(ResponseCode::BAD_REQUEST, "unsupported direction received");
    }
}

inline typename PicoStepperMotor::Shoulder str_to_shoulder(const std::string& shoulder) {
    static const std::map<std::string, PicoStepperMotor::Shoulder> mapping {
        {"a0", PicoStepperMotor::Shoulder::A0},
        {"a1", PicoStepperMotor::Shoulder::A1},
        {"b0", PicoStepperMotor::Shoulder::B0},
        {"b1", PicoStepperMotor::Shoulder::B1}
    };
    const auto iter(mapping.find(shoulder));
    if (mapping.end() == iter) {
        throw ServerException(ResponseCode::BAD_REQUEST, "bad shoulder tag received: " + shoulder);
    }
    return iter->second;
}

inline StepperMotor *create_stepper_motor(const Data& create_cfg) {
    const auto& cfg_obj(Data::cast<Object>(create_cfg));
    PicoStepperMotor::ShouldersMapping shoulders;
    Data::cast<Object>(create_cfg).for_each(
        [&shoulders](const std::string& shoulder_tag, const Data& id) {
            if ("en" == shoulder_tag) {
                return;
            }
            shoulders.insert({str_to_shoulder(shoulder_tag), static_cast<unsigned int>(Data::cast<Integer>(id).get())});
        }
    );
    const auto en(static_cast<unsigned int>(Data::cast<Integer>(cfg_obj.access("en")).get()));
    return new PicoStepperMotor(shoulders, en);
}

inline void timeout(const LinearMovement::TimeUnit& timeout_us) {
    sleep_us(timeout_us);
}