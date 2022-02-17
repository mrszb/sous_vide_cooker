#include "main.h"
#include "temperature_sensor.h"
#include "delay_us.h"
#include "gpio_mode.h"

TemperatureSensor::TemperatureSensor(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin):
_GPIOx(GPIOx), 
_GPIO_Pin(GPIO_Pin)
{
}

void TemperatureSensor::set_data_pin_as_output(void)
{
	GPIO_SetPinAsOutput(_GPIOx, _GPIO_Pin);
}

void TemperatureSensor::set_data_pin_as_input(void)
{
	GPIO_SetPinAsInput(_GPIOx, _GPIO_Pin);
}

void TemperatureSensor::pull_data_pin_down(void)
{
	set_data_pin_as_output();
	HAL_GPIO_WritePin (_GPIOx, _GPIO_Pin, GPIO_PIN_RESET);
}

bool TemperatureSensor::is_conversion_done(void)
{
	set_data_pin_as_input();
	return  (HAL_GPIO_ReadPin (_GPIOx, _GPIO_Pin) == GPIO_PIN_SET);
}

void TemperatureSensor::wait_conversion(void)
{
	set_data_pin_as_input();
	bool continues = true;
	while (continues)
	{
 		continues = HAL_GPIO_ReadPin (_GPIOx, _GPIO_Pin) == GPIO_PIN_RESET;
		delay_us(10);
	}
}

bool TemperatureSensor::start (void)
{
	pull_data_pin_down();
	delay_us (480);   

	set_data_pin_as_input();
	delay_us (80);   

	// detecting presence pulse
	bool ok = HAL_GPIO_ReadPin (_GPIOx, _GPIO_Pin) == GPIO_PIN_RESET;
	delay_us (400);

	return ok;
}

void TemperatureSensor::write (uint8_t data)
{
	for (int i=0; i<8; i++)
	{
		if ((data & (1<<i))!=0) 
		{
			// must be released within 15us
			pull_data_pin_down();
			delay_us (1);  

			set_data_pin_as_input();
			delay_us (40); 
		}
		else
		{
			//must hold at least 60us
			pull_data_pin_down();
			delay_us (60); 
			set_data_pin_as_input();
		}
	}
}

uint8_t TemperatureSensor::read (void)
{
	uint8_t value=0;

	for (int i=0; i<8; i++)
	{
		// min 2us recovery
		pull_data_pin_down();
		delay_us (2); 

		set_data_pin_as_input();
		// let sensor drive pin now ...
		if (HAL_GPIO_ReadPin (_GPIOx, _GPIO_Pin) == GPIO_PIN_SET) 
			value |= 1<<i;
		// total duration min 60us
		delay_us (60); 
	}
	return value;
}





