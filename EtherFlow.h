
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
#include "Socket.h"
#include "SlotManager.h"


class Socket;
class SlotManager;



class EtherFlow
{
	friend class Socket;
	friend class SlotManager;

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


	static uint16_t checksum(uint16_t sum, const uint8_t *data, uint16_t len, bool &carry, bool& odd);
	static uint16_t checksum(uint16_t sum, const uint8_t *data, uint16_t len);

	static void sendIPPacket(uint8_t headerLength);



private:

	static void writeByte(byte b);
	static void writeByte(uint16_t dst, byte b);
	static void writeBuf(uint16_t dst, uint16_t len, const byte* data);
	static void writeBuf(uint16_t len, const byte* data);
	static void moveMem(uint16_t dest, uint16_t src, uint16_t len);

	static void packetSend(uint16_t len);
	static void packetSend(uint16_t len, const byte* data);

	static bool processChunk(uint8_t& handler, uint16_t len);
	static bool processTCPSegment(bool isHeader, uint16_t len);

public: static uint16_t packetReceiveChunk();
	static void makeWhoHasARPRequest(IPAddress& ip);
	static void makeARPReply();
	static void processARPReply();
	static void tick();
	static void registerSocket(Socket* socket);
	static void unregisterSocket(Socket*);

	static Socket* currentSocket;
	
	
public:	
	static Socket* sockets[MAX_TCP_SOCKETS];
	

	
};

typedef EtherFlow eth;

#endif
