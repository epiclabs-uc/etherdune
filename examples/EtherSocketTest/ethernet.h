
// PIN Connections (Using Arduino UNO):
//   VCC -   3.3V
//   GND -    GND
//   SCK - Pin 13
//   SO  - Pin 12
//   SI  - Pin 11
//   CS  - Pin  8
//
#define __PROG_TYPES_COMPAT__

#ifndef ESEthernet_h
#define ESEthernet_h


#if ARDUINO >= 100
#include <Arduino.h> // Arduino 1.0
#else
#include <WProgram.h> // Arduino 0022
#endif

#include <avr/pgmspace.h>
#include "esenc28j60.h"
#include "net.h"
#include "inet.h"

class EtherSocket : public ENC28J60 
{
private:
	MACAddress* mymac;
	EthBuffer buffer;

public:
	uint8_t begin(MACAddress& mac, uint8_t cspin);
	void loop();



};

extern EtherSocket eth; //!< Global presentation of EtherSocket class

#endif
