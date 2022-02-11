#include <cinttypes>
#include "main.h"

class keypad
{

public:
	keypad() = default;
	uint8_t raw_scan();
};