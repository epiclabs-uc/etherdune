// Socket.h

#ifndef _SOCKET_h
#define _SOCKET_h

#include <ACross/ACross.h>

#include "inet.h"
#include "config.h"
#include "SharedBuffer.h"
#include "NetworkService.h"

class Socket;


class Socket : private NetworkService
{
	friend class EtherFlow;

 protected:


	 uint8_t stateTimer;
	 uint8_t state;
	 uint32_t sequenceNumber;
	 uint32_t ackNumber;

	 SharedBuffer buffer;
	

private:
	
	static uint8_t srcPort_L_count;
	bool sendAck;
	
	bool processHeader();
	bool processData(uint16_t len, uint8_t* data);


	void processOutgoingBuffer();
	uint16_t calcPseudoHeaderChecksum(uint8_t protocol, uint16_t length);

	void prepareTCPPacket(bool options, uint16_t dataLength);
	void prepareIPPacket();
	void releaseWindow(int32_t& bytesAck);
	void calcTCPChecksum(bool options, uint16_t dataLength, uint16_t dataChecksum);

	void tick();
	void sendSYN();
	void sendFIN();

	void setState(uint8_t newState, uint8_t timeout);

	void printState();


 public:
	 nint16_t remotePort;
	 nint16_t localPort;
	 IPAddress remoteAddress;

	 Socket();	 
	 void connect();
	 uint16_t write(uint16_t len, const byte* data);
	 uint16_t send(uint16_t len, const byte* data);
	 void close();
	 void terminate();
	 

	 virtual void onConnect();
	 virtual void onClose();
	 virtual void onReceive(uint16_t len, const byte* data);






};




#endif

