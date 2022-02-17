#include "main.h"
#include "lcd.h"

// fonts from https://www.eevblog.com/forum/microcontrollers/stm32-nokia-5110-large-font-issue/
#include "font.h"
#include <cstring>


#define SPI_TIMEOUT_MS 5

// 84x48 screen
// there are 6 horizontal lines on the screen
// each line 14 characters, 6 pixels wide
// line is 8 pixels tall

#define LCD_CHAR_WIDTH 6
#define LCD_CHAR_HEIGHT 8

#define LCD_NOF_LINES (48 / LCD_CHAR_HEIGHT)
#define LCD_LINE_LENGTH (84 / LCD_CHAR_WIDTH)

const uint8_t _Zeros[84 * 48 / 8 ] = {0x00};
const uint8_t _Pattern[84 * 48 / 8] = {0x0f};

LCD_Screen::LCD_Screen(SPI_HandleTypeDef* spi,
			GPIO_TypeDef* rstPort, uint16_t rstPin,
			GPIO_TypeDef* cePort, uint16_t cePin,
			GPIO_TypeDef* dcPort, uint16_t dcPin):
				_spi(spi),
				_rstPort(rstPort), _rstPin(rstPin),
				_cePort(cePort), _cePin(cePin),
				_dcPort(dcPort), _dcPin(dcPin),
			_vertical_addressing(false)
{}

inline void LCD_Screen::reset()
{
	HAL_GPIO_WritePin(_rstPort, _rstPin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(_rstPort, _rstPin, GPIO_PIN_SET);
}

inline void LCD_Screen::to_commands_mode()
{
	HAL_GPIO_WritePin(_dcPort, _dcPin, GPIO_PIN_RESET);
}

inline void LCD_Screen::to_data_mode()
{
	HAL_GPIO_WritePin(_dcPort, _dcPin, GPIO_PIN_SET);
}

inline void LCD_Screen::data_transfer_start()
{
	HAL_GPIO_WritePin(_cePort, _cePin, GPIO_PIN_RESET);
}

inline void LCD_Screen::data_transfer_end()
{
	HAL_GPIO_WritePin(_cePort, _cePin, GPIO_PIN_SET);
}

HAL_StatusTypeDef LCD_Screen::send(const uint8_t *data, uint16_t Size)
{
	// hack
	uint8_t *pData = const_cast<uint8_t*>(data);
	HAL_StatusTypeDef stat = HAL_SPI_Transmit(_spi, pData, Size, SPI_TIMEOUT_MS);
	if (stat != HAL_OK) {
		reset();
	}
	return stat;
}

HAL_StatusTypeDef LCD_Screen::send_command(uint8_t cmd)
{
	to_commands_mode();
	return send (&cmd, sizeof(cmd));
}

HAL_StatusTypeDef LCD_Screen::set_function_set(bool pd, bool v, bool h)
{
	uint8_t command = (1 << 5) |
			(pd ? (1 << 2) : 0) |
			(v  ? (1 << 1) : 0) |
			(h ?  1 : 0);
	return send_command(command);
}

HAL_StatusTypeDef LCD_Screen::set_YX_address_of_RAM (uint8_t y, uint8_t x)
{
	// 01000YYY
	// 1XXXXXXX
	uint8_t cmds[] =
	{
		uint8_t((1 << 6) | (y & 0x07)),
		uint8_t((1 << 7) | (x & 0x7f))
	};
	return send(cmds, 2);
}

HAL_StatusTypeDef LCD_Screen::set_temperature_control(uint8_t tc)
{
	uint8_t cmd = (1 << 2) | (tc & 0x03);
	return send (&cmd, sizeof(cmd));
}

HAL_StatusTypeDef LCD_Screen::set_bias_system(uint8_t bs)
{
	uint8_t cmd = (1 << 4) | (bs & 0x07);
	return send (&cmd, sizeof(cmd));
}

HAL_StatusTypeDef LCD_Screen::set_vop(uint8_t vop)
{
	uint8_t cmd = (1 << 7) | (vop & 0x7f);
	return send(&cmd, sizeof(cmd));
}

HAL_StatusTypeDef LCD_Screen::set_display_control(uint8_t d, uint8_t e)
{
	uint8_t cmd = (1 << 3) | ((d & 0x01) << 2) | (e & 0x01);
	return send (&cmd, sizeof(cmd));
}

HAL_StatusTypeDef LCD_Screen::write_bytes_at_position (
	uint8_t ixVer, uint8_t ixHor,
	 const uint8_t* bytes, uint16_t size)
{
	to_commands_mode();
	data_transfer_start();
	set_function_set(/*power_down*/ false, _vertical_addressing, 0);
	set_YX_address_of_RAM(ixVer, LCD_CHAR_WIDTH*ixHor);
	data_transfer_end();

	to_data_mode();
	data_transfer_start();
	HAL_StatusTypeDef stat = send(bytes, size);
	data_transfer_end();

	return stat;
}

HAL_StatusTypeDef LCD_Screen::clear()
{
	return write_bytes_at_position(0, 0, _Zeros, sizeof(_Zeros));
}

void LCD_Screen::init()
{
	reset();

	data_transfer_start();
	set_function_set(/*power_down*/ false , _vertical_addressing, 1);

	set_bias_system(3);
	set_vop(0x40);
	set_temperature_control(2);

	set_function_set(/*power_down*/ false , _vertical_addressing, 0);
	set_display_control(1, 0);
	set_YX_address_of_RAM(0, 0);
	data_transfer_end();

	clear();
}

HAL_StatusTypeDef LCD_Screen::write_line(uint8_t ixVertical, const std::string& txt)
{
	static_assert(LCD_LINE_LENGTH*LCD_CHAR_WIDTH == 84);
	uint8_t line_buffer [LCD_LINE_LENGTH*LCD_CHAR_WIDTH] = { 0 };
	std::string line = txt.substr(0, LCD_LINE_LENGTH);

	size_t ix = 0;
	for (char c : line)
	{
		size_t idx = size_t(c);
		memcpy(&line_buffer[LCD_CHAR_WIDTH*ix], font6_8[idx-0x20], LCD_CHAR_WIDTH);
		ix++;
	}

	return write_bytes_at_position(ixVertical, /*x*/ 0, line_buffer, sizeof(line_buffer));
}

