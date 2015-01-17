
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
#include "net.h"
#include "inet.h"
#include "enc28j60constants.h"
#include "config.h"

static const uint8_t ARP_TABLE_LENGTH = 2;
static const int16_t MAX_ARP_TTL = 20 * 60; // 20 mins


class EtherSocket
{
public:
	static MACAddress localMAC;
	static IPAddress localIP;
	static bool broadcast_enabled; //!< True if broadcasts enabled (used to allow temporary disable of broadcast for DHCP or other internal functions)
	static EthBuffer chunk;


public:
	static uint8_t begin(uint8_t cspin);
	static void loop();
	

	static void staticSetup(IPAddress & ip);

	static MACAddress* whoHas(IPAddress& ip);

	static void enableBroadcast(bool temporary = false);

	static bool isLinkUp();


private:

	static void processChunk(bool isHeader, uint16_t len);
	static uint16_t packetReceiveChunk();
	static void makeWhoHasARPRequest(IPAddress& ip);
	static void processARPReply();
	static void tick();

};

typedef EtherSocket eth;

#endif
