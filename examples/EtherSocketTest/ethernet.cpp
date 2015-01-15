

#include "ethernet.h"
#include <stdarg.h>
#include <avr/eeprom.h>

EtherSocket eth;


uint8_t EtherSocket::begin(MACAddress& mac, uint8_t cspin)
{
	mymac = &mac;
	return initialize(sizeof(buffer), mac.b, cspin);

}


void EtherSocket::loop()
{

}