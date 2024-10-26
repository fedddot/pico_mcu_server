#include "gpio.hpp"
#include "pico_stepper_motor.hpp"

using namespace pico_mcu_platform;

using Shoulder = typename PicoStepperMotor::Shoulder;
using GpioState = typename manager::Gpio::State;

const PicoStepperMotor::States pico_mcu_platform::PicoStepperMotor::s_states {
	PicoStepperMotor::State(
		{
			{Shoulder::A1,	GpioState::HIGH},
			{Shoulder::A0,	GpioState::LOW},
			{Shoulder::B1,	GpioState::LOW},
			{Shoulder::B0,	GpioState::LOW}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::A1,	GpioState::HIGH},
			{Shoulder::A0,	GpioState::LOW},
			{Shoulder::B1,	GpioState::HIGH},
			{Shoulder::B0,	GpioState::LOW}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::A1,	GpioState::LOW},
			{Shoulder::A0,	GpioState::LOW},
			{Shoulder::B1,	GpioState::HIGH},
			{Shoulder::B0,	GpioState::LOW}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::A1,	GpioState::LOW},
			{Shoulder::A0,	GpioState::HIGH},
			{Shoulder::B1,	GpioState::HIGH},
			{Shoulder::B0,	GpioState::LOW}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::A1,	GpioState::LOW},
			{Shoulder::A0,	GpioState::HIGH},
			{Shoulder::B1,	GpioState::LOW},
			{Shoulder::B0,	GpioState::LOW}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::A1,	GpioState::LOW},
			{Shoulder::A0,	GpioState::HIGH},
			{Shoulder::B1,	GpioState::LOW},
			{Shoulder::B0,	GpioState::HIGH}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::A1,	GpioState::LOW},
			{Shoulder::A0,	GpioState::LOW},
			{Shoulder::B1,	GpioState::LOW},
			{Shoulder::B0,	GpioState::HIGH}
		}
	),
	PicoStepperMotor::State(
		{
			{Shoulder::A1,	GpioState::HIGH},
			{Shoulder::A0,	GpioState::LOW},
			{Shoulder::B1,	GpioState::LOW},
			{Shoulder::B0,	GpioState::HIGH}
		}
	)
};
