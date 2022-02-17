#include "main.h"
#include "gpio_mode.h"

void GPIO_SetPinAsInput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	for (uint8_t i = 0x00; i < 0x10; i++) {
		if (GPIO_Pin & (1 << i)) {
			// 00 input
			GPIOx->MODER &= ~(0x03 << (2 * i));
		}
	}
}

void GPIO_SetPinAsOutput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	for (uint8_t i = 0x00; i < 0x10; i++) {
		if (GPIO_Pin & (1 << i)) {
			// 01 output
			GPIOx->MODER = (GPIOx->MODER & ~(0x03 << (2 * i))) | (0x01 << (2 * i));
		}
	}
}

void GPIO_SetPinAsAF(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	for (uint8_t i = 0x00; i < 0x10; i++) {
		if (GPIO_Pin & (1 << i)) {
			// 01 output
			GPIOx->MODER = (GPIOx->MODER & ~(0x03 << (2 * i))) | (0x10 << (2 * i));
		}
	}
}
