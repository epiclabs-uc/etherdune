// Socket.h

#ifndef _SOCKET_h
#define _SOCKET_h

#include <ACross.h>

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
	void prepareIPPacket();


 public:
	 nint16_t remotePort;
	 nint16_t localPort;
	 IPAddress remoteIP;

	 uint16_t write(uint16_t len, const byte* data);
	 uint16_t write(const String& s);
	 uint16_t write(const __FlashStringHelper* pattern, ...);
	 template <class T>
	 inline uint16_t write(const T& message)
	 {
		 return write(sizeof(T), (byte*)&message);
	 }


};




#endif

