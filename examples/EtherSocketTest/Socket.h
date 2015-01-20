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
#include "ethernet.h"

class Socket;

typedef void(*SocketCallback)(Socket* socket, uint8_t eventType);

class Socket
{
	friend class EtherSocket;
 protected:

	 nint16_t dstPort;
	 uint8_t srcPort_L;
	 IPAddress dstAddr;
	 SocketCallback eventHandler;
	 uint8_t id;
	 uint8_t state;
	 
	 uint32_t slotMap;
	 uint8_t numSlots;
	 
	 uint8_t currentSlot;
	 uint8_t currentSlotPos;

	 uint32_t sequenceNumber;
	

private:
	
	static uint8_t srcPort_L_count;

	void tick();
	

 public:

	 Socket(SocketCallback eventHandlerCallback = NULL);	 
	 ~Socket();
	 void connect(IPAddress& ip, uint16_t port);






};




#endif

