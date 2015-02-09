// Socket.h

#ifndef _SOCKET_h
#define _SOCKET_h

#include <ACross/ACross.h>

#include "inet.h"
#include "config.h"
#include "SharedBuffer.h"
#include "NetworkService.h"

class Socket;


class Socket : protected NetworkService
{

 protected:

	 SharedBuffer buffer;

	static uint8_t srcPort_L_count;
	uint16_t calcPseudoHeaderChecksum(uint8_t protocol, uint16_t length);

	void prepareIPPacket();


 public:
	 nint16_t remotePort;
	 nint16_t localPort;
	 IPAddress remoteAddress;


};




#endif

