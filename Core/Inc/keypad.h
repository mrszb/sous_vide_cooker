#pragma once 

#include <cinttypes>
#include <string>
#include "button_debouncer.h"

enum Keys
{
	KEY_RIGHT = 0,
	KEY_LEFT,
	KEY_DOWN,
	KEY_UP,

	//
	NUM_OF_KEYS
};

typedef uint32_t SYSTEM_TIME;

struct KeyPadEvent
{
	Keys key;
	ButtonEvent evt;
	SYSTEM_TIME tm;

	std::string to_debug_string() const;
};


#define KeyPadQLen  10

class KeyPad
{
	ButtonDebouncer debouncer[NUM_OF_KEYS];

	KeyPadEvent evtQueue[KeyPadQLen];
	KeyPadEvent *pQOut,*pQIn;

public:
	KeyPad();
	uint8_t raw_scan();

	// needs to be called in regular intervals .. 1ms suggested
	void update(SYSTEM_TIME tm);

	bool get_next_event(KeyPadEvent* evt);
};
