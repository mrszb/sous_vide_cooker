#include "main.h"
#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <iostream>

#include "keypad.h"
#include "delay_us.h"
#include "temperature_sensor.h"
#include "lcd.h"
#include "gpio_mode.h"
#include <cassert>

#include "PID_v1.h"
#undef LIBRARY_VERSION
#include "PID_AutoTune_v0.h"

#include "WProgram.h"
#include "pid_tester.h"

#define FW_RAW_BLINKY 1
#define	FW_STATE_MACHINE_BLINKY  2
#define FW_PID_UI 3

//#define _FW FW_STATE_MACHINE_BLINKY
#define _FW FW_PID_UI

#if _FW == FW_STATE_MACHINE_BLINKY
	#include "sm_blinky.hpp"
#elif _FW == FW_PID_UI
	#include "sm_pid_ui.hpp"
#endif


static bool _use_sleep_mode = false;

// TIM10 running at 1MHZ for us delays
// TIM11 periodic scanning keyboard

extern TIM_HandleTypeDef htim2;
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

//////////////////////////////////////////////////


struct PidParams
{
	double Kp;
	double Ki;
	double Kd;
};

PidParams _pidParams = {10, 0.5, 0};

double _setpoint;
double _inputPid = 0;
double _outputPid = 0;

PID _pid (&_inputPid, &_outputPid, &_setpoint,
		_pidParams.Kp, _pidParams.Ki, _pidParams.Kd, DIRECT);

double _atuneStep=500;
double _atuneNoise=1;
unsigned int _atuneLookback=20;

bool tuning = false;

PID_ATune aTune(&_inputPid, &_outputPid);
SYSTEM_TIME _time_window = 3000;

void adjust_cooker_duty_cycle(int dc)
{
	TIM3->CCR3 = dc * 10000 / 100;
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

	//TIM2->CCR1 = dc * 1000 / 100;
	//HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

void lcd_screen_on(int dc)
{
	TIM2->CCR1 = dc * 10000 / 100;
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

//////////////////////////////////////////////////////
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

#if _FW == FW_PID_UI

void parameter_screen(const char* name, double& ref)
{
	HAL_GPIO_WritePin (LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_SET);

	lcd.write_line(0, name);

	std::ostringstream out;
	out << " " << ref;
	lcd.write_line(1, out.str());

	for (int i = 2; i < 6; i++)
		lcd.write_line(i, "");
}

void refresh_screen(SCREEN_STYLE style)
{
	switch (style)
	{
		case SCREEN_OFF:
		{
			GPIO_SetPinAsOutput(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin);
			HAL_GPIO_WritePin (LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET);

			for (int i = 0; i < 6; i++)
				lcd.write_line(i, "");
			break;
		}

		case SCREEN_SETPOINT:
		{
			HAL_GPIO_WritePin (LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_SET);

			lcd.write_line(0, "Setpoint:");

			std::ostringstream out;
			out << " " << _setpoint << "C";
			lcd.write_line(1, out.str());

			for (int i = 2; i < 6; i++)
				lcd.write_line(i, "");
			break;
		}

		case SCREEN_RUN:
		{
			HAL_GPIO_WritePin (LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_SET);

			lcd.write_line(0, "Run:");

			std::ostringstream out;
			out << " " << _setpoint;

			switch (_pid.GetMode())
			{
				case MANUAL:
					lcd.write_line(1, "MANUAL");
					{
						std::ostringstream out;
						out << " " << _inputPid << "C";
						lcd.write_line(2, out.str());
					}
					break;

				case AUTOMATIC:
					lcd.write_line(1, "AUTO");
					{
						std::ostringstream out;
						out << " " << _inputPid << "C";
						lcd.write_line(2, out.str());
					}
					{
						std::ostringstream out;
						out << " " << _outputPid << "%";
						lcd.write_line(3, out.str());
					}
					break;
			}

			break;
		}

		case SCREEN_SET_P:
			parameter_screen("Kp", _pidParams.Kp);
			break;

		case SCREEN_SET_I:
			parameter_screen("Ki", _pidParams.Ki);
			break;

		case SCREEN_SET_D:
			parameter_screen("Kd", _pidParams.Kd);
			break;

	}
}


const void run_screen()
{
	lcd.write_line(0, "Running");

	{
		std::ostringstream out;
		out << " " << _setpoint;
		lcd.write_line(1, out.str());
	}

	{
		std::ostringstream out;
		out << "Kp: " << _pidParams.Kp;
		lcd.write_line(2, out.str());
	}

	{
		std::ostringstream out;
		out << " " << _setpoint;
		lcd.write_line(3, out.str());
	}

	{
		std::ostringstream out;
		out << " " << _setpoint;
		lcd.write_line(4, out.str());
	}
}

void change_param(const char* name, int dir)
{
	if (std::string(name) == "SP")
	{
		if (dir > 0)
			_setpoint += 1.0;
		else if (dir < 0)
			_setpoint -= 1.0;
	}

	if (std::string(name) == "P")
	{
		if (dir > 0)
			_pidParams.Kp += 0.5;
		else if (dir < 0)
			_pidParams.Kp -= 0.5;

		if (_pidParams.Kp < 0.0)
			_pidParams.Kp = 0.0;

		_pid.SetTunings(_pidParams.Kp, _pidParams.Ki, _pidParams.Kd);
	}

	if (std::string(name) == "I")
	{
		if (dir > 0)
			_pidParams.Ki += 0.5;
		else if (dir < 0)
			_pidParams.Ki -= 0.5;

		if (_pidParams.Ki < 0.0)
			_pidParams.Ki = 0.0;

		_pid.SetTunings(_pidParams.Kp, _pidParams.Ki, _pidParams.Kd);
	}
	if (std::string(name) == "D")
	{
		if (dir > 0)
			_pidParams.Kd += 0.5;
		else if (dir < 0)
			_pidParams.Kd -= 0.5;

		if (_pidParams.Kd < 0.0)
			_pidParams.Kd = 0.0;

		_pid.SetTunings(_pidParams.Kp, _pidParams.Ki, _pidParams.Kd);
	}

	if (std::string(name) == "RUN")
	{
		if (dir > 0)
			_pid.SetMode(AUTOMATIC);
		else if (dir < 0)
			_pid.SetMode(MANUAL);
	}
}

#endif


extern "C" void appmain (void)
{
	HAL_TIM_Base_Start_IT(&htim11);
	HAL_TIM_Base_Start_IT(&htim10);

	lcd.init();
	lcd.clear();

	lcd.write_line(0, "Hello");
	lcd.write_line(1, "  World");

	//PID_experiment();

	using namespace sml;

#if _FW == FW_STATE_MACHINE_BLINKY
	Blinky_PWM();

	sm<state_machine_blinky> sm;
	assert(sm.is("led pwm"_s));

#elif _FW == FW_PID_UI
	_setpoint = 20;

	sm<state_machine_pid_ui> sm;
	assert(sm.is("OFF"_s));

	lcd_screen_on(60);

	// ReInit_PWM_CookerPin();
	TIM3->CCR3 = 0 * 10000 / 100;
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

	const int duty_cycle_lcd_backlight = 100;
	TIM2->CCR1 = duty_cycle_lcd_backlight  * 10000 / 100;
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

#endif

	bool has_temperature_sensor = tempsensor.start();
	if (has_temperature_sensor)
	{
		// start temperature measurement
		tempsensor.write (tempsensor.CMD_SKIP_ROM);
		tempsensor.write (tempsensor.CMD_CONVERT_T);
	}

	_pid.SetOutputLimits(0, 100);
	_pid.SetMode(MANUAL);

	SYSTEM_TIME _last_time_event = 0;

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
			}
		}

	   int elapsed = tm -_last_time_event;
	   if (elapsed >= 1000)
	   {
		   _last_time_event = tm;

			if (has_temperature_sensor)
			{

				if (tempsensor.is_conversion_done())
				{
					tempsensor.start ();
					tempsensor.wait_conversion();
					tempsensor.write (tempsensor.CMD_SKIP_ROM);
					tempsensor.write (tempsensor.CMD_READ_SCRATCHPAD);

					auto b1 = tempsensor.read();
					auto b2 = tempsensor.read();

					uint16_t temp = (b2 << 8) |  b1;
					auto t = static_cast<double>(temp)/16.0;
					//std::cout << "->" << "temp: " << t << "C" << std::endl;

					_inputPid = t;
					if (sm.is("RUN"_s))
						refresh_screen(SCREEN_RUN);
				}

				if (_pid.Compute())
				{
					adjust_cooker_duty_cycle(_outputPid);

					if (sm.is("RUN"_s))
						refresh_screen(SCREEN_RUN);

					int ms = tm % 1000;
					std::cout << int(tm/1000) << "."
							<< std::setfill('0') << std::setw(3) << (ms % 100) << " " << _setpoint << " " << _inputPid << " " << _outputPid << "%" << std::endl;
				}

				tempsensor.start();
				tempsensor.write (tempsensor.CMD_SKIP_ROM);
				tempsensor.write (tempsensor.CMD_CONVERT_T);
	   	   }
		}

	}
}


