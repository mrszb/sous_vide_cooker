#include "main.h"
#include <string>
#include <cstring>
#include <iostream>

#include "keypad.h"

extern TIM_HandleTypeDef htim3;

SYSTEM_TIME tm = 0;
KeyPad keys;

extern "C" void on_periodic_timer (void)
{
	keys.update(tm++);
}

extern "C" void appmain (void)
{
	HAL_TIM_Base_Start_IT(&htim3);
	while (1)
	{

		KeyPadEvent ev;
		bool has_event = keys.get_next_event(&ev);
		if (has_event)
		{
			std::cout << ev.to_debug_string() << std::endl;

		 	if (ev.evt == BUTTON_PRESSED)
			{
		 		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		 		HAL_GPIO_TogglePin(COOKER_GPIO_Port, COOKER_Pin);
			}
		}
	}
}


