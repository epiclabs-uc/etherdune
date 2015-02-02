// Checksum.h

#ifndef _CHECKSUM_h
#define _CHECKSUM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
#include <inttypes.h>
#endif


class Checksum
{
public:
	static uint16_t add(uint16_t a, uint16_t b);
	static uint16_t add(uint16_t a, uint16_t b, bool odd);
	static uint16_t calc(uint16_t len, const uint8_t *data);
	static uint16_t calc(uint16_t checksum, uint16_t len, const uint8_t *data);

};


#endif

