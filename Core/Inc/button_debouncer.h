#pragma once

#define DebounceQLen  16

enum ButtonEvent
{
	BUTTON_NO_CHANGE = -1,
	BUTTON_PRESSED,
	BUTTON_RELEASED,
};


class ButtonDebouncer
{
	bool DebounceCircularQueue[DebounceQLen],
		*p_debounce;

	bool button_pressed;
	int running_total;

public:
	ButtonDebouncer ();
	ButtonEvent update_with_sampling(bool sample);

	bool is_button_pressed();
};




