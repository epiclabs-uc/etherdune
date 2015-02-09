#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

#include <ACross/ACross.h>

#include "Socket.h"

class TCPSocket;


class TCPSocket : public Socket
{
	
protected:

	uint32_t sequenceNumber;
	uint32_t ackNumber;
	uint8_t stateTimer;
	uint8_t state;

private:

	bool sendAck;

	bool processHeader();
	bool processData(uint16_t len, uint8_t* data);

	void processOutgoingBuffer();

	void prepareTCPPacket(bool options, uint16_t dataLength);
	void releaseWindow(int32_t& bytesAck);
	void calcTCPChecksum(bool options, uint16_t dataLength, uint16_t dataChecksum);

	void tick();
	void sendSYN();
	void sendFIN();

	void setState(uint8_t newState, uint8_t timeout);

	void printState();


public:

	TCPSocket();
	void connect();
	uint16_t write(uint16_t len, const byte* data);
	void close();
	void terminate();

	virtual void onConnect();
	virtual void onClose();
	virtual void onReceive(uint16_t len, const byte* data);

};



#endif
