#include "button_debouncer.h"

#define LowerThr (DebounceQLen/3)
#define UpperThr (DebounceQLen/3)*2

ButtonDebouncer::ButtonDebouncer()
{
	p_debounce = DebounceCircularQueue;
	running_total = 0;
	button_pressed = false;
}

bool ButtonDebouncer::is_button_pressed()
{
	return button_pressed;
}

ButtonEvent ButtonDebouncer::update_with_sampling(bool btn_fresh_sample)
{
	// debouncing routine
	// point to last recorded sample, overwrite with new sample ...
	bool btn_oldest_sample = *p_debounce;
	*p_debounce = btn_fresh_sample;

	// rotate pointer forward
	p_debounce ++;
	if (p_debounce >= DebounceCircularQueue + DebounceQLen)
	    p_debounce = DebounceCircularQueue;

	if (btn_fresh_sample != btn_oldest_sample)
	{
		// if sample out not same as sample in we need to update
		// running total
		if (btn_fresh_sample)
			running_total++;
		else
			running_total--;
	}

	ButtonEvent newEvt = BUTTON_NO_CHANGE;

	if  (button_pressed)
	{
		if (running_total < LowerThr)
		{
			button_pressed = false;
			// BTN UP EVENT
			newEvt = BUTTON_RELEASED;
		}
	}
	else
	{
		if (running_total > UpperThr)
		{
			button_pressed = true;
			// BTN DOWN EVENT
			newEvt = BUTTON_PRESSED;
		}
	}

	return newEvt;
}



