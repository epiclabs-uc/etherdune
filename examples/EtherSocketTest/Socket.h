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


class Socket
{
	friend class EtherSocket;
 protected:


	 uint8_t srcPort_L;
	 uint8_t retries;

	 
	 uint8_t id;
	 uint8_t state;
	 
	 uint32_t slotMap;
	 uint8_t numSlots;
	 
	 uint8_t currentSlot;
	 uint8_t currentSlotPos;

	 uint32_t sequenceNumber;
	

private:
	
	static uint8_t srcPort_L_count;
	bool processSegment(bool isHeader);

	void tick();
	void sendSYN();

 public:
	 nint16_t remotePort;
	 IPAddress remoteAddress;

	 Socket();	 
	 ~Socket();
	 void connect();
	 uint16_t write(uint16_t len, const byte* data);
	 

	 virtual void onConnect();
	 virtual void onClose();






};




#endif

