/*
 * appmain.cpp
 *
 */

#include "main.h"
#include <string>
#include <cstring>
#include <iostream>
#include "keypad.h"

void raw_scan_blinky (void)
{

	KeyPad keys;
	while (1)
	{
		uint8_t pressed = keys.raw_scan();
		if (pressed)
		{
			 HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			 HAL_GPIO_TogglePin(COOKER_GPIO_Port, COOKER_Pin);
		 	 std::cout << int(pressed) << std::endl;
		}

		 HAL_Delay(100);
	}
}


