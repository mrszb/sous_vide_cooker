#include "main.h"
#include <string>
#include <cstring>
#include <iostream>

#include "keypad.h"
#include "delay_us.h"
#include "temperature_sensor.h"
#include "lcd.h"

#include "sml.hpp"

#include "PID_v1.h"
#include "PID_AutoTune_v0.h"

#include "WProgram.h"
#include "pid_tester.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim10;
extern SPI_HandleTypeDef hspi3;

SYSTEM_TIME tm = 0;
KeyPad keys;

unsigned long millis()
{
	return tm;
}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim3)
	{
		keys.update(tm++);
	}
}

extern "C" void appmain (void)
{
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim10);

	TemperatureSensor tempsensor(TEMP_DATA_GPIO_Port, TEMP_DATA_Pin);

	LCD_Screen lcd(&hspi3,
	  LCD_RST_GPIO_Port, LCD_RST_Pin,
	  LCD_SCE_GPIO_Port, LCD_SCE_Pin,
	  LCD_DC_GPIO_Port, LCD_DC_Pin);

	lcd.init();
	lcd.clear();
	lcd.write_line(0, "TESTING PID");
	lcd.write_line(1, " |");
	lcd.write_line(2, " |");
	lcd.write_line(3, " |");
	lcd.write_line(4, "START");

	pid_test_setup();
	while (pid_test_loop())
		;

	lcd.write_line(3, "-|-");
	lcd.write_line(4, "END");

	while (1)
	{
		KeyPadEvent ev;
		bool has_event = keys.get_next_event(&ev);
		if (has_event)
		{
			std::cout << ev.to_debug_string() << std::endl;

		 	if (ev.evt == BUTTON_PRESSED)
			{
		 		bool sensor_found = tempsensor.start();
		 		if (sensor_found)
		 		{
					HAL_Delay (1);
					tempsensor.write (tempsensor.CMD_SKIP_ROM);
					tempsensor.write (tempsensor.CMD_CONVERT_T); 
					tempsensor.wait_conversion();

					tempsensor.start ();
					HAL_Delay(1);
					tempsensor.write (tempsensor.CMD_SKIP_ROM);
					tempsensor.write (tempsensor.CMD_READ_SCRATCHPAD); 

					auto b1 = tempsensor.read();
					auto b2 = tempsensor.read();

					uint16_t temp = (b2 << 8) |  b1;
					auto t = static_cast<double>(temp)/16.0;
					std::cout << "->" << t << "C" << std::endl;
		 		}

		 		for (int i = 0; i < 10; i++)
		 		{
		 			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		 			HAL_GPIO_TogglePin(COOKER_GPIO_Port, COOKER_Pin);

		 			delay_us(50000);
		 		}
			}
		}
	}
}


