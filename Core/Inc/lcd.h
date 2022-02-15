#pragma once
#include "stm32f4xx_hal_spi.h"
#include <string>

class LCD_Screen
{
	SPI_HandleTypeDef* _spi;

	GPIO_TypeDef* _rstPort;
	uint16_t _rstPin;
	GPIO_TypeDef* _cePort;
	uint16_t _cePin;
	GPIO_TypeDef* _dcPort;
	uint16_t _dcPin;

	void reset();
	void to_commands_mode() ;
	void to_data_mode();
	void data_transfer_start() ;
	void data_transfer_end();

	HAL_StatusTypeDef send(const uint8_t *pData, uint16_t Size);
	HAL_StatusTypeDef send_command(uint8_t);

	HAL_StatusTypeDef set_function_set(bool pd, bool v, bool h);
	HAL_StatusTypeDef set_YX_address_of_RAM(uint8_t y, uint8_t x);
	HAL_StatusTypeDef set_display_control(uint8_t d, uint8_t e);

	HAL_StatusTypeDef write_bytes_at_position(
			uint8_t ixVer, uint8_t ixHor,
	        const uint8_t* bytes, uint16_t size);

	HAL_StatusTypeDef set_temperature_control(uint8_t tc);
	HAL_StatusTypeDef set_bias_system(uint8_t bs);
	HAL_StatusTypeDef set_vop(uint8_t vop);

	bool _vertical_addressing;

public:


	LCD_Screen(SPI_HandleTypeDef* spi,
			GPIO_TypeDef* rstPort, uint16_t rstPin,
			GPIO_TypeDef* cePort, uint16_t cePin,
			GPIO_TypeDef* dcPort, uint16_t dcPin);

	void init();
	HAL_StatusTypeDef clear();

	// writes complete line
	HAL_StatusTypeDef write_line(uint8_t ixVertical, const std::string& txt);
};
