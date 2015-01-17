// Socket.h

#ifndef _SOCKET_h
#define _SOCKET_h

#include "inet.h"


#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class Socket;

typedef void(*SocketCallback)(Socket* socket, uint8_t eventType);

class Socket
{
 protected:

	 nint16_t dstPort;
	 IPAddress dstAddr;
	 SocketCallback eventHandler;
	 uint8_t id;

private:
	Socket();

 public:

	 
	 void connect(IPAddress& ip, uint16_t port);

	 static void begin();
	 static Socket* create(SocketCallback eventHandlerCallback);


};


#endif

