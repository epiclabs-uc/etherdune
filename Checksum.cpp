// 
// 
// 

#include "Checksum.h"

uint16_t Checksum::add(uint16_t a, uint16_t b)
{
	a += b;

	if (a<b)
		a++;

	return a;
}

uint16_t Checksum::add(uint16_t a, uint16_t b, bool odd)
{
	if (odd)
		b = (b << 8) | (b >> 8);

	return add(a, b);

}


uint16_t Checksum::calc(uint16_t len, const uint8_t *data)
{
	uint16_t t;
	uint16_t sum = 0;

	const uint16_t *dataptr;
	const uint16_t *last;

	dataptr = (uint16_t*)data;
	last = dataptr + (len >> 1);

	while (dataptr < last)
	{
		sum = add(sum, *dataptr);
		dataptr++;
	}

	if (len & 1)
		sum = add(sum, *((uint8_t*)dataptr));

	return sum;

}
