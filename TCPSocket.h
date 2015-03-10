#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

#include <ACross.h>

#include "Socket.h"
#include "Stateful.h"

class TCPSocket;


class TCPSocket : public Socket, public Stateful
{

private:

	uint32_t sequenceNumber;
	uint32_t ackNumber;
	TCPFlags nextFlags;

	bool onPacketReceived();
	void processOutgoingBuffer();
	void prepareTCPPacket(bool options, uint16_t dataLength);
	void releaseWindow(int32_t& bytesAck);
	void tick();
	void sendSYN(bool ack);
	__FlashStringHelper* getStateString();

public:

	TCPSocket();

	void connect();
	void close();
	void terminate();
	void listen();
	void accept();
	void accept(TCPSocket& listener);
	void push();

	virtual void onConnect();
	virtual void onClose();
	virtual void onReceive(uint16_t len, const byte* data);
	virtual void onTerminate();

};



#endif
