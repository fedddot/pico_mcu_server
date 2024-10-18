#include <stdexcept>
#include <string>

#include "pico/stdio.h"

#include "data.hpp"
#include "gpio.hpp"
#include "gpio_manager.hpp"
#include "in_memory_inventory.hpp"
#include "integer.hpp"
#include "json_request_parser.hpp"
#include "json_response_serializer.hpp"
#include "movement_manager.hpp"
#include "pico_gpi.hpp"
#include "pico_gpo.hpp"
#include "pico_stepper_motor.hpp"
#include "pico_synchronous_ipc_connection.hpp"
#include "request.hpp"
#include "resources_vendor.hpp"
#include "server.hpp"
#include "server_types.hpp"
#include "stepper_motor.hpp"
#include "stepper_motor_manager.hpp"
#include "string.hpp"

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

static PicoSynchronousIpcConnection::Baud cast_baud(uint baud);

static bool match(const RawData& data, const RawData& head, const RawData& tail);
static Request extract(RawData *data, const RawData& head, const RawData& tail);
static RawData serialize(const server::Response& response, const RawData& head, const RawData& tail);

static Gpio *create_gpio(const Body& create_body);
static StepperMotor *create_stepper_motor(const Body& create_body);
static Body read_stepper_motor(const StepperMotor& motor);

class ClonableWrapper: public ResourcesVendor::ClonableManager {
public:
    ClonableWrapper(Manager *origin): m_manager(origin) {
        if (!origin) {
            throw std::invalid_argument("invalid origin ptr received");
        }
    }
    ClonableWrapper(const ClonableWrapper&) = default;
    ClonableWrapper& operator=(const ClonableWrapper&) = default;
    
    void create_resource(const Body& create_request_body) override {
        m_manager->create_resource(create_request_body);
    }
    Body read_resource(const Path& route) const override {
        return m_manager->read_resource(route);
    }
    Body read_all_resources() const override {
        return m_manager->read_all_resources();
    }
    void update_resource(const Path& route, const Body& update_request_body) override {
        m_manager->update_resource(route, update_request_body);
    }
    void delete_resource(const Path& route) override {
        m_manager->delete_resource(route);
    }
    bool contains(const Path& route) const override {
        return m_manager->contains(route);
    }
    Manager *clone() const override {
        return new ClonableWrapper(*this);
    }
private:
    std::shared_ptr<Manager> m_manager;
};

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

    InMemoryInventory<ResourceId, Gpio> gpio_inventory;
    InMemoryInventory<ResourceId, StepperMotor> stepper_motor_inventory;

    ResourcesVendor vendor;
    vendor.add_manager(
        "gpios",
        ClonableWrapper(
            new GpioManager(
                &gpio_inventory,
                create_gpio
            )
        )
    );
    vendor.add_manager(
        "steppers",
        ClonableWrapper(
            new StepperMotorManager(
                &stepper_motor_inventory,
                create_stepper_motor,
                read_stepper_motor
            )
        )
    );
    vendor.add_manager(
        "movements",
        ClonableWrapper(new MovementManager(&stepper_motor_inventory))
    );

    Server<std::string> server(
        &connection,
        "cnc_server",
        &vendor
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

inline Gpio *create_gpio(const Body& create_body) {
    auto id = Data::cast<String>(create_body.access("id")).get();
    auto dir = static_cast<Gpio::Direction>(Data::cast<Integer>(create_body.access("dir")).get());
    switch (dir) {
    case Gpio::Direction::IN:
        return new PicoGpi(std::stoi(id));
    case Gpio::Direction::OUT:
        return new PicoGpo(std::stoi(id));
    default:
        throw std::invalid_argument("unsupported direction received");
    }
}

inline StepperMotor *create_stepper_motor(const Body& create_body) {
    auto a0 = static_cast<unsigned int>(Data::cast<Integer>(create_body.access("a0")).get());
    auto a1 = static_cast<unsigned int>(Data::cast<Integer>(create_body.access("a1")).get());
    auto b0 = static_cast<unsigned int>(Data::cast<Integer>(create_body.access("b0")).get());
    auto b1 = static_cast<unsigned int>(Data::cast<Integer>(create_body.access("b1")).get());
    auto en = static_cast<unsigned int>(Data::cast<Integer>(create_body.access("en")).get());
    return new PicoStepperMotor(
        PicoStepperMotor::ShouldersMapping(
            {
                {PicoStepperMotor::Shoulder::A0, a0},
                {PicoStepperMotor::Shoulder::A1, a1},
                {PicoStepperMotor::Shoulder::B0, b0},
                {PicoStepperMotor::Shoulder::B1, b1},
            }
        ),
        en
    );
}

inline Body read_stepper_motor(const StepperMotor& motor) {
    (void)motor;
    return Body();
}