/*
 * keypad.cpp
 *
 */
#include "keypad.h"
#include "button_debouncer.h"
#include "main.h"
#include <string>


KeyPad::KeyPad()
{
	pQIn  = evtQueue;
    pQOut = evtQueue;
}

uint8_t KeyPad::raw_scan()
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

uint8_t KeyPad::scan()
{
	uint8_t keys = 0;
	for (size_t key_ix=0; key_ix < keypad::NUM_OF_KEYS; key_ix++)
	{
		if (debouncer[key_ix].is_button_pressed())
			keys |=  (1 << key_ix);
	}

	return keys;
}

std::string KeyPadEvent::to_debug_string() const
{
	std::string s;
	switch (key)
	{
		case keypad::KEY_RIGHT:
			s += "[RIGHT]";
			break;

		case keypad::KEY_LEFT:
			s += "[LEFT ]";
			break;

		case keypad::KEY_UP:
			s += "[UP   ]";
			break;

		case keypad::KEY_DOWN:
			s += "[DOWN ]";
			break;

		default:
			s += "[!!!!unknown!!!!]";
			break;
	}

	s += " ";

    switch (evt)
    {
		case BUTTON_PRESSED:
			s += "pressed";
			break;

		case BUTTON_RELEASED:
			s += "released";
			break;

		case BUTTON_NO_CHANGE:
		default:
			assert(0);

    }

	s+= " tm: ";
	s+= std::to_string(tm);

	return s;
}

void KeyPad::update(SYSTEM_TIME system_time)
{
	uint8_t last_scan = raw_scan();
	for (size_t key_ix=0; key_ix < keypad::NUM_OF_KEYS; key_ix++)
	{
		auto evt = debouncer[key_ix].update_with_sampling(last_scan & (1 << key_ix));
		if (evt != BUTTON_NO_CHANGE)
		{	
			// if buttons recognized at transitioned up/down
			// queue keypad event:
			//
			KeyPadEvent padevt;
			padevt.key = (keypad::Keys) key_ix;
			padevt.evt = evt;
			padevt.tm = system_time;

			// insert into keypad event queue
			*pQIn = padevt;

			KeyPadEvent *pNextQIn = (pQIn >= evtQueue+KeyPadQLen-1) ?
		    	evtQueue : pQIn+1;

			if (pNextQIn != pQOut)
				pQIn = pNextQIn;
		}

	}
}


bool KeyPad::get_next_event(KeyPadEvent* evt)
{
	if (pQOut == pQIn)
		return false;

	*evt = *pQOut;
	pQOut++;

	if (pQOut >= evtQueue + KeyPadQLen)
		pQOut = evtQueue;

	return true;
}




