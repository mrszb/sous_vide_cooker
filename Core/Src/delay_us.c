#include "main.h"
#include "delay_us.h"

extern TIM_HandleTypeDef htim10;

void delay_us (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim10,0);
	while (__HAL_TIM_GET_COUNTER(&htim10) < us);
}
