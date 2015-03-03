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




#include <ACross.h>

#include "net.h"
#include "inet.h"
#include "enc28j60constants.h"
#include "config.h"
#include "NetworkService.h"


class Socket;
class SharedBuffer;
class NetworkService;
class TCPSocket;
class UDPSocket;
class DMATest;
class EtherFlow 
{
	friend class Socket;
	friend class SharedBuffer;
	friend class NetworkService;
	friend class TCPSocket;
	friend class UDPSocket;
	friend class DMATest;

public:

	static bool broadcast_enabled; //!< True if broadcasts enabled (used to allow temporary disable of broadcast for DHCP or other internal functions)
	


public:
	static uint8_t begin(uint8_t cspin);
	static void loop();
	


	
	static void enableBroadcast(bool temporary = false);
	static bool isLinkUp();


	



private:

	static void writeByte(byte b);
	static void writeByte(uint16_t dst, byte b);
	static void writeBuf(uint16_t dst, uint16_t len, const byte* data);
	static void writeBuf(uint16_t len, const byte* data);
	static uint16_t hardwareChecksum(uint16_t src, uint16_t len);
	static uint16_t hardwareChecksumRxOffset(uint16_t offset, uint16_t len);
	static void moveMem(uint16_t dest, uint16_t src, uint16_t len);
	static void readBuf(uint16_t src, uint16_t len, byte* data);
	static void readBuf(uint16_t len, byte* data);
	static byte readByte(uint16_t src);

	static void packetSend(uint16_t len);
	static void packetSend(uint16_t len, const byte* data);

	static void loadSample();
	static void loadAll();
	static void release();

public: 



	

	
};

#endif
