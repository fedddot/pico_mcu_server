#include "pico_stepper_motor.hpp"
#include "gpio.hpp"

using namespace pico_mcu_platform;

using Shoulder = typename PicoStepperMotor::Shoulder;
using GpioState = typename manager::Gpio::State;

const PicoStepperMotor::States pico_mcu_platform::PicoStepperMotor::s_states {
	PicoStepperMotor::State(
		{
			{Shoulder::LEFT_HIGH,	GpioState::HIGH},
			{Shoulder::LEFT_LOW,	GpioState::LOW},
			{Shoulder::RIGHT_HIGH,	GpioState::LOW},
			{Shoulder::RIGHT_LOW,	GpioState::LOW}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::LEFT_HIGH,	GpioState::HIGH},
			{Shoulder::LEFT_LOW,	GpioState::LOW},
			{Shoulder::RIGHT_HIGH,	GpioState::HIGH},
			{Shoulder::RIGHT_LOW,	GpioState::LOW}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::LEFT_HIGH,	GpioState::LOW},
			{Shoulder::LEFT_LOW,	GpioState::LOW},
			{Shoulder::RIGHT_HIGH,	GpioState::HIGH},
			{Shoulder::RIGHT_LOW,	GpioState::LOW}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::LEFT_HIGH,	GpioState::LOW},
			{Shoulder::LEFT_LOW,	GpioState::HIGH},
			{Shoulder::RIGHT_HIGH,	GpioState::HIGH},
			{Shoulder::RIGHT_LOW,	GpioState::LOW}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::LEFT_HIGH,	GpioState::LOW},
			{Shoulder::LEFT_LOW,	GpioState::HIGH},
			{Shoulder::RIGHT_HIGH,	GpioState::LOW},
			{Shoulder::RIGHT_LOW,	GpioState::LOW}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::LEFT_HIGH,	GpioState::LOW},
			{Shoulder::LEFT_LOW,	GpioState::HIGH},
			{Shoulder::RIGHT_HIGH,	GpioState::LOW},
			{Shoulder::RIGHT_LOW,	GpioState::HIGH}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::LEFT_HIGH,	GpioState::LOW},
			{Shoulder::LEFT_LOW,	GpioState::LOW},
			{Shoulder::RIGHT_HIGH,	GpioState::LOW},
			{Shoulder::RIGHT_LOW,	GpioState::HIGH}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::LEFT_HIGH,	GpioState::HIGH},
			{Shoulder::LEFT_LOW,	GpioState::LOW},
			{Shoulder::RIGHT_HIGH,	GpioState::LOW},
			{Shoulder::RIGHT_LOW,	GpioState::HIGH}
		}
	)
};

const PicoStepperMotor::State pico_mcu_platform::PicoStepperMotor::s_shutdown_state(
	{
		{Shoulder::LEFT_HIGH,	GpioState::LOW},
		{Shoulder::LEFT_LOW,	GpioState::LOW},
		{Shoulder::RIGHT_HIGH,	GpioState::LOW},
		{Shoulder::RIGHT_LOW,	GpioState::LOW}
	}
);