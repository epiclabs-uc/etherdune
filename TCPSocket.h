#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

#include <ACross.h>

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

	void tick();
	void sendSYN(bool ack);
	void sendFIN();

	void setState(uint8_t newState, uint8_t timeout);

	__FlashStringHelper* getStateString();


public:

	TCPSocket();
	void connect();

	void close();
	void terminate();
	void listen();
	void accept();
	void accept(TCPSocket& listener);

	virtual void onConnect();
	virtual void onClose();
	virtual void onReceive(uint16_t len, const byte* data);

};



#endif
