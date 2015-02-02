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
#include "SharedBuffer.h"
#include "EtherFlow.h"

class Socket;


class Socket : public SharedBuffer
{
	friend class EtherFlow;

 protected:


	 uint8_t srcPort_L;
	 uint8_t stateTimer;

	 
	 uint8_t state;
	 


	 uint32_t sequenceNumber;
	 uint32_t ackNumber;
	

private:
	
	static uint8_t srcPort_L_count;
	bool sendAck;
	bool processSegment(bool isHeader, uint16_t len);
	void processOutgoingBuffer();
	void preparePacket(bool options, uint16_t dataLength);
	void releaseWindow(int32_t& bytesAck);
	void calcTCPChecksum(bool options, uint16_t dataLength, uint16_t dataChecksum);

	void tick();
	void sendSYN();
	void sendFIN();

	void setState(uint8_t newState, uint8_t timeout);

	void printState();


 public:
	 nint16_t remotePort;
	 IPAddress remoteAddress;

	 Socket();	 
	 ~Socket();
	 void connect();
	 uint16_t write(uint16_t len, const byte* data);
	 void close();
	 void terminate();
	 

	 virtual void onConnect();
	 virtual void onClose();
	 virtual void onReceive(uint16_t len, const byte* data);






};




#endif

