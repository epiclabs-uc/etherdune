#ifndef __UDPSOCKET_H_
#define __UDPSOCKET_H_

#include "Socket.h"


class UDPSocket : public Socket
{

public:

	IPAddress remoteIP;
	nint16_t remotePort;
	nint16_t localPort;
	SharedBuffer buffer;

	void write(uint16_t length, uint8_t* data);
	void send();

	virtual void onReceive(uint16_t fragmentLength, uint16_t totalLength, const byte* data);



};







#endif