#pragma once
#include "stm32f4xx_hal_gpio.h"



class TemperatureSensor
{
	GPIO_TypeDef* _GPIOx;
	uint16_t _GPIO_Pin;

	void set_data_pin_as_output(void);
	void set_data_pin_as_input(void);
	void pull_data_pin_down(void);

public:

	static const uint8_t CMD_READ_ROM = 0x33;
	static const uint8_t CMD_SKIP_ROM = 0xcc;
	static const uint8_t CMD_CONVERT_T = 0x44;
	static const uint8_t CMD_READ_SCRATCHPAD = 0xbe;

	TemperatureSensor(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
	bool start();
	void wait_conversion();

	void write (uint8_t data);
	uint8_t read ();
};


