/*
 * appmain.cpp
 *
 */

#include "main.h"
#include <string>
#include <cstring>


extern "C" void appmain (void)
{
	while (1)
	{
		 volatile GPIO_PinState userbtn = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

		 if (userbtn == GPIO_PIN_RESET)
			 HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

		 std::string s= "Hello!\r\n";
		 uint8_t buf[20];
		 std::copy(s.begin(), s.end(), buf);
		 //HAL_UART_Transmit(&huart2, buf, strlen((char*)buf), HAL_MAX_DELAY);
		 HAL_Delay(100);
	}
}


