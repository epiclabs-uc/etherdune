#ifndef __UDPSOCKET_H_
#define __UDPSOCKET_H_

#include "Socket.h"


class UDPSocket : public Socket
{
private:
	bool processHeader();
	bool processData(uint16_t len, uint8_t* data);
	
	bool sending;

public:

	UDPSocket();

	nint16_t remotePort;
	nint16_t localPort;
	SharedBuffer buffer;

	void write(uint16_t length, uint8_t* data);
	bool send();
	void tick();

	virtual void onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data);



};







#endif