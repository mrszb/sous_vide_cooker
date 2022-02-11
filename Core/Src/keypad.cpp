/*
 * keypad.cpp
 *
 */
#include "keypad.h"

uint8_t keypad::raw_scan()
{
	 volatile GPIO_PinState btn_up = HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin);
	 volatile GPIO_PinState btn_down = HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin);
	 volatile GPIO_PinState btn_right = HAL_GPIO_ReadPin(BTN_RIGHT_GPIO_Port, BTN_RIGHT_Pin);
	 volatile GPIO_PinState btn_left = HAL_GPIO_ReadPin(BTN_LEFT_GPIO_Port, BTN_LEFT_Pin);

	 return  (btn_up == GPIO_PIN_RESET) ? 1 << 3 : 0
			 + (btn_down == GPIO_PIN_RESET) ? 1 << 2 : 0
			 + (btn_left == GPIO_PIN_RESET) ? 1 << 1 : 0
			 +  (btn_right == GPIO_PIN_RESET) ? 1  : 0;
}

