#ifndef ESEthernet_h
#define ESEthernet_h

// PIN Connections (Using Arduino UNO):
//   VCC -   3.3V
//   GND -    GND
//   SCK - Pin 13
//   SO  - Pin 12
//   SI  - Pin 11
//   CS  - Pin  8
//
#define __PROG_TYPES_COMPAT__




#include <ACross/ACross.h>

#include "net.h"
#include "inet.h"
#include "enc28j60constants.h"
#include "config.h"
#include "Socket.h"



class Socket;
class SlotManager;
class SharedBuffer;


class EtherFlow 
{
	friend class Socket;
	friend class SharedBuffer;


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


	static void sendIPPacket(uint8_t headerLength);



private:

	static void writeByte(byte b);
	static void writeByte(uint16_t dst, byte b);
	static void writeBuf(uint16_t dst, uint16_t len, const byte* data);
	static void writeBuf(uint16_t len, const byte* data);
	static void moveMem(uint16_t dest, uint16_t src, uint16_t len);
	static void readBuf(uint16_t src, uint16_t len, byte* data);
	static void readBuf(uint16_t len, byte* data);
	static byte readByte(uint16_t src);

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
