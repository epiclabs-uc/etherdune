#ifndef __UDPSOCKET_H_
#define __UDPSOCKET_H_

#include "Socket.h"


class UDPSocket : public Socket
{

private:

	bool onPacketReceived();
	bool sending;

protected:

	void prepareUDPPacket(uint16_t dataLength, uint16_t dataChecksum);

	virtual bool sendPacket();

public:

	UDPSocket();

	bool send();
	void tick();

	virtual void onReceive(uint16_t len);

};







#endif