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
	// state machine for the keypad demo
	#include "sm_blinky.hpp"

#elif _FW == FW_PID_UI
	// project state machine is defined here
	#include "sm_pid_ui.hpp"
#endif

// set this to true if you want the CPU go to sleep between
// 1 ms periodic events
// CPU current goes from about 17mA to 5mA
static bool _use_sleep_mode = true;

// We need timers !
// TIM2 used for PWM driving LCD backlight
// TIM3 used for PWM driving cooker and red LED
// TIM10 running at 1MHZ for us delays
// TIM11 periodic scanning keyboard

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;

// SPI3 (master only) talks to LCD screen
extern SPI_HandleTypeDef hspi3;

// external peripherals:
// temperature sensor is bit-banged one-wire prtocol on TEMP_DATA pin
TemperatureSensor tempsensor(TEMP_DATA_GPIO_Port, TEMP_DATA_Pin);

// define LCD screen pins
// its SPI and few extra pins to select, reset and select data/command mode
LCD_Screen lcd(&hspi3,
  LCD_RST_GPIO_Port, LCD_RST_Pin,
  LCD_SCE_GPIO_Port, LCD_SCE_Pin,
  LCD_DC_GPIO_Port, LCD_DC_Pin);

// system time starts at 0 on cold reset
// incremented by one by periodic interrupt (Timer11)
SYSTEM_TIME tm = 0;

// keypad consists of 4 debounced buttons
//
//        UP
// RIGHT       LEFT
//       DOWN
//
// (these are periodically scanned)
KeyPad keys;

// this will be set by button external interrupt
volatile bool blue_button_triggered;

// arduino style function to keep PID library happy
unsigned long millis()
{
	return tm;
}

// Timer 11 does do job of incrementing internal time
// rescan keypad (debounce and generate keyboard press/release events)
// if we put system to sleep ... it wakes it up within 1ms
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// called each ms
	if (htim == &htim11)
	{
		keys.update(tm++);
	}
}

// calback from the interrupt
// blue buttons can wake up system even if he put it in sleep and stop
// periodic updates ...
extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == B1_Pin)
	{
		blue_button_triggered = true;
	}
}

extern "C" void ReInit_PWM_CookerPin(void);

//////////////////////////////////////////////////

// here we store PID parameters
struct PidParams
{
	double Kp;
	double Ki;
	double Kd;
};

PidParams _pidParams = {10, 0.5, 0};

// PID user sets setpoint
// PID algo will monitor input (temperature inside of cooker in C)
// and suggest output in range 0-100(%) to drive heating element

double _setpoint;
double _inputPid = 0;
double _outputPid = 0;

PID _pid (&_inputPid, &_outputPid, &_setpoint,
		_pidParams.Kp, _pidParams.Ki, _pidParams.Kd, DIRECT);


// autotune should improve the vals (had not use it yet)
// should be activated near stable state though

double _atuneStep=500;
double _atuneNoise=1;
unsigned int _atuneLookback=20;

bool tuning = false;

PID_ATune aTune(&_inputPid, &_outputPid);
SYSTEM_TIME _time_window = 3000;


// adjusting the cooker duty cycle is done here
// currently I am using very long cycle, almost as if I was using mechanical switch
// I can see better the activity

void adjust_cooker_duty_cycle(int dc)
{
	TIM3->CCR3 = dc * 10000 / 100;
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

	//TIM2->CCR1 = dc * 1000 / 100;
	//HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

// LCD screen is attached to PWM output
// no success with it - screen flicker 
// (I am using crrently this pin
// as set/reset GPIO output instead)

void lcd_screen_on(int dc)
{
	TIM2->CCR1 = dc * 10000 / 100;
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

//////////////////////////////////////////////////////
// provided with library was a simulation
// it mocks input and switches setpoints
// it was an interesting way to see if system is ready for the library
// could be put back as easer egg 

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

// next few routines
// (?)
// are Model View for keyboard/screen interaction
// control provided by state machine

void parameter_screen(const char* name, double& ref)
{
	// turn on LCD backlight
	HAL_GPIO_WritePin (LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_SET);

	// write parameter name and value
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

// manipulate parameters up/down
// (called by the state machine)

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

////////////////////////////////////////////////////////////////////////////

extern "C" void appmain (void)
{
	HAL_TIM_Base_Start_IT(&htim11);
	HAL_TIM_Base_Start_IT(&htim10);

	lcd.init();
	lcd.clear();

	lcd.write_line(0, "Hello");
	lcd.write_line(1, "  World");

	// interesting test - maybe I put it as easter egg sequence
	// PID_experiment();
	// it simulates imaginary system responses if you have none

	using namespace sml;

#if _FW == FW_STATE_MACHINE_BLINKY
	Blinky_PWM();

	sm<state_machine_blinky> sm;
	assert(sm.is("led pwm"_s));

#elif _FW == FW_PID_UI
	// initialixe state machine
	// setpoint should be low enough to deactivate any heating attempts
	_setpoint = 20;

	sm<state_machine_pid_ui> sm;
	assert(sm.is("OFF"_s));

	lcd_screen_on(60);

	// ReInit_PWM_CookerPin();
	TIM3->CCR3 = 0 * 10000 / 100;
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

	// trying to drive backlight as PWM
	// it did not work - will be switched on/off in state machine
	// instead
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

	// PID output is 0-100%
	// its always non-negative, we have no cooler
	// only heater to run at 0-100% power
	_pid.SetOutputLimits(0, 100);

	// manual means PID controller does not modify output
	_pid.SetMode(MANUAL);

	// initialize system milisec timer 
	SYSTEM_TIME _last_time_event = 0;

	while (1)
	{
		// You might want to sleep between events
		// periodic interrupt will wake you up ...
		if (_use_sleep_mode)
			HAL_PWR_EnterSLEEPMode (
					PWR_LOWPOWERREGULATOR_ON,
					PWR_SLEEPENTRY_WFI);

		// wake up by external interrupt
		// if this after hybernation, it should restore timers and outputs 
		// not implemented yet

		if (blue_button_triggered)
		{
			blue_button_triggered = false;
			std::cout << "->blue button interrupt " << std::endl;
			sm.process_event(blue_button{});
		}

		// any events (pressed/released) from keypad
		KeyPadEvent ev;
		bool has_event = keys.get_next_event(&ev);
		if (has_event)
		{
			std::cout << "->key " << ev.to_debug_string() << std::endl;

			// pres buttons are sent to state machine for action
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

		// another reason we could be here is because system woke up by 1ms periodic timer
		// ... mostly ignored but ...
		// every sec or so we would like to measure temperature and call the PID controler
		// to respond with approriate output
		int elapsed = tm -_last_time_event;

		// every second
		if (elapsed >= 1000)
		{
			_last_time_event = tm;

			if (has_temperature_sensor)
			{
				// we started temperature measurement at some time
				// lets check if it is done and results are ready

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
					// if pid adjusted output
					//       need to tell the cooker
					adjust_cooker_duty_cycle(_outputPid);

					// and refresh RUN screen 
					if (sm.is("RUN"_s))
						refresh_screen(SCREEN_RUN);

					// send PID status to serial for debugging
					int ms = tm % 1000;
					std::cout << int(tm/1000) << "."
							<< std::setfill('0') << std::setw(3) << (ms % 100) << " " << _setpoint << " " << _inputPid << " " << _outputPid << "%" << std::endl;
				}

				// schedule/start new temperature measurement
				tempsensor.start();
				tempsensor.write (tempsensor.CMD_SKIP_ROM);
				tempsensor.write (tempsensor.CMD_CONVERT_T);
			}
		}

	}
}


