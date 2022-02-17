#include "main.h"
#include <string>
#include <cstring>
#include <iostream>

#include "keypad.h"
#include "delay_us.h"
#include "temperature_sensor.h"
#include "lcd.h"
#include "gpio_mode.h"

#include "sml.hpp"
namespace sml = boost::sml;

#include <cassert>

#include "PID_v1.h"
#undef LIBRARY_VERSION
#include "PID_AutoTune_v0.h"

#include "WProgram.h"
#include "pid_tester.h"

static bool _use_sleep_mode = false;

// TIM10 running at 1MHZ for us delays
// TIM11 periodic scanning keyboard

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;
extern SPI_HandleTypeDef hspi3;

// external peripherals:
TemperatureSensor tempsensor(TEMP_DATA_GPIO_Port, TEMP_DATA_Pin);

LCD_Screen lcd(&hspi3,
  LCD_RST_GPIO_Port, LCD_RST_Pin,
  LCD_SCE_GPIO_Port, LCD_SCE_Pin,
  LCD_DC_GPIO_Port, LCD_DC_Pin);

SYSTEM_TIME tm = 0;
KeyPad keys;

volatile bool blue_button_triggered;

unsigned long millis()
{
	return tm;
}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// called each ms
	if (htim == &htim11)
	{
		keys.update(tm++);
	}
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == B1_Pin)
	{
		blue_button_triggered = true;
	}
}

extern "C" void ReInit_PWM_CookerPin(void);


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
}

int32_t CH3_DC = 25;

void Blinky_PWM()
{
	ReInit_PWM_CookerPin();
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
	TIM3->CCR3 = CH3_DC;
}

void Blinky_ON()
{
	// you can change duty cycle
	// TIM3->CCR3 = 100;
	// or turn it on:

	GPIO_SetPinAsOutput(COOKER_GPIO_Port, COOKER_Pin);
	HAL_GPIO_WritePin (COOKER_GPIO_Port, COOKER_Pin, GPIO_PIN_SET);
}

void Blinky_OFF()
{
	GPIO_SetPinAsOutput(COOKER_GPIO_Port, COOKER_Pin);
	HAL_GPIO_WritePin (COOKER_GPIO_Port, COOKER_Pin, GPIO_PIN_RESET);
}

extern void PID_simulation();

void PID_experiment()
{
	lcd.write_line(0, "TESTING PID");
	lcd.write_line(1, " |");
	lcd.write_line(2, " |");
	lcd.write_line(3, " |");
	lcd.write_line(4, "START");
	PID_simulation();
	lcd.write_line(3, "-|-");
	lcd.write_line(4, "END");
}

namespace sml = boost::sml;

namespace {

struct key_up {};
struct key_down {};
struct key_left {};
struct key_right {};

struct blue_button{};
struct timeout{};

// nonsense just try guard:
const auto is_key_down_valid = [](const key_down&) { return true; };

const auto turn_on = [] { Blinky_ON(); std::cout << "action: turning on" << std::endl; };
const auto turn_off = [] { Blinky_OFF(); std::cout << "action: turning off" << std::endl;};
const auto turn_pwm = [] { Blinky_PWM(); std::cout << "action: pwm" << std::endl;};

struct state_machine_blinky {
  auto operator()() const {

    using namespace sml;
    return make_transition_table(

      *"led pwm"_s + event<key_up> / turn_on = "led on"_s
	  ,"led pwm"_s + event<key_down> / turn_off = "led off"_s
	  ,"led off"_s + event<key_up> / turn_on = "led on"_s
	  ,"led on"_s + event<key_down> / turn_off = "led off"_s
      ,"led off"_s + event<key_right>  / turn_pwm = "led pwm"_s
	  ,"led on"_s + event<key_right> / turn_pwm = "led pwm"_s

	  ,"led pwm"_s + sml::on_entry<_> / [] { std::cout << "pwm entry" << std::endl; }
	  ,"led pwm"_s + sml::on_exit<_>  / [] { std::cout << "pwm exit" << std::endl; }

      ,"timed wait"_s + event<timeout> / turn_off = X
    );
  }
};
}

extern "C" void appmain (void)
{
	HAL_TIM_Base_Start_IT(&htim11);
	HAL_TIM_Base_Start_IT(&htim10);

	lcd.init();
	lcd.clear();

	lcd.write_line(0, "Hello");
	lcd.write_line(1, "  World");

	//PID_experiment();
	Blinky_PWM();

	using namespace sml;

	sm<state_machine_blinky> sm;
	assert(sm.is("led pwm"_s));

	bool sensor_found = tempsensor.start();

	while (1)
	{
		// You might want to sleep between events
		// periodic interrupt will wake you up ...
		if (_use_sleep_mode)
			HAL_PWR_EnterSLEEPMode (
					PWR_LOWPOWERREGULATOR_ON,
					PWR_SLEEPENTRY_WFI);

		if (blue_button_triggered)
		{
			blue_button_triggered = false;
			std::cout << "->blue button interrupt " << std::endl;
			sm.process_event(blue_button{});
		}

		KeyPadEvent ev;
		bool has_event = keys.get_next_event(&ev);
		if (has_event)
		{
			std::cout << "->key " << ev.to_debug_string() << std::endl;

		 	if (ev.evt == BUTTON_PRESSED)
			{

		 		switch (int(ev.key))
		 		{
		 			case keypad::KEY_RIGHT:
		 				sm.process_event(key_right{});
		 				break;

		 			case keypad::KEY_LEFT:
		 				sm.process_event(key_left{});
		 				break;

		 			case keypad::KEY_DOWN:
		 				sm.process_event(key_down{});
		 				break;

		 			case keypad::KEY_UP:
		 				sm.process_event(key_up{});
		 				break;

		 			default:
		 				break;

		 		}

		 		if (sensor_found)
		 		{
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
					std::cout << "->" << "temp: " << t << "C" << std::endl;
		 		}

			}
		}
	}
}


