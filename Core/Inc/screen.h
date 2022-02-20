
#ifndef INC_SCREEN_H_
#define INC_SCREEN_H_

enum SCREEN_STYLE
{
	SCREEN_OFF = 0,
	SCREEN_SETPOINT = 1,
	SCREEN_RUN,
	SCREEN_SET_P,
	SCREEN_SET_I,
	SCREEN_SET_D
};

void refresh_screen(SCREEN_STYLE style);

#endif /* INC_SCREEN_H_ */
