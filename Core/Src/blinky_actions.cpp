#include "blinky_actions.h"
#include "main.h"
#include "gpio_mode.h"

extern void adjust_cooker_duty_cycle(int dc);
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim2;

extern "C" void ReInit_PWM_CookerPin(void);

///////////////////////////

/*
void Blinky_toggling()
{
	for (int i = 0; i < 5; i++)
	{
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		HAL_GPIO_TogglePin(COOKER_GPIO_Port, COOKER_Pin);
		delay_us(50000);
	}
}

void Blinky_PWM_pulsing()
{
	int32_t CH3_DC = 0;
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

	for (int i =0; i < 5; i++)
	{
		while(CH3_DC < 100)
		{
			TIM3->CCR3 = CH3_DC;
			CH3_DC += 10;
			HAL_Delay(100);
		}
		while(CH3_DC > 0)
		{
			TIM3->CCR3 = CH3_DC;
			CH3_DC -= 10;
			HAL_Delay(100);
		}
	}
}*/

int32_t CH3_DC = 25;

void Blinky_PWM()
{
	ReInit_PWM_CookerPin();
	TIM3->CCR3 = CH3_DC * 10000 / 100;
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

void Blinky_ON()
{
	// you can change duty cycle
	// TIM3->CCR3 = 100;
	// or turn it on:

	GPIO_SetPinAsOutput(COOKER_GPIO_Port, COOKER_Pin);
	HAL_GPIO_WritePin (COOKER_GPIO_Port, COOKER_Pin, GPIO_PIN_SET);

	GPIO_SetPinAsOutput(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin);
	HAL_GPIO_WritePin (LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_SET);
}

void Blinky_OFF()
{
	GPIO_SetPinAsOutput(COOKER_GPIO_Port, COOKER_Pin);
	HAL_GPIO_WritePin (COOKER_GPIO_Port, COOKER_Pin, GPIO_PIN_RESET);

	GPIO_SetPinAsOutput(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin);
	HAL_GPIO_WritePin (LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET);
}

