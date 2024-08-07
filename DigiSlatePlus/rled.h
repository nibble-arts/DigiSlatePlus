#ifndef RLED_H
#define RLED_H


#include <Arduino.h>


class RLED {
public:
	void begin(uint16_t);
	void on(void);
	void off(void);
	void toggle(void);
	void flash(void);
	void flash(uint16_t);
	void flash(uint16_t, uint8_t);

private:
	uint16_t _port;
};

#endif