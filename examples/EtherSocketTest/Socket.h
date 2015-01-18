// Socket.h

#ifndef _SOCKET_h
#define _SOCKET_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "inet.h"
#include "config.h"

class Socket;

typedef void(*SocketCallback)(Socket* socket, uint8_t eventType);

class Socket
{
 protected:

	 nint16_t dstPort;
	 uint8_t srcPort_L;
	 IPAddress dstAddr;
	 SocketCallback eventHandler=NULL;
	 uint8_t id;
	 uint8_t state = 0;

	

private:
	
	 Socket();
	static Socket sockets[MAX_TCP_SOCKETS];
	static uint16_t activeSockets;

 public:

	 
	 void connect(IPAddress& ip, uint16_t port);
	 void release();

	 static void begin();

	 static Socket* create(SocketCallback eventHandlerCallback = NULL);


};




#endif

