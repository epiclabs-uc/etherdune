#ifndef __UDPSOCKET_H_
#define __UDPSOCKET_H_
#include <ACross.h>
#include "NetworkService.h"
#include "SharedBuffer.h"

class UDPSocket : public NetworkService
{

private:
	bool processHeader();
	bool processData(uint16_t len, uint8_t* data);


	bool sending;

protected:
	void prepareUDPPacket(uint16_t dataLength, uint16_t dataChecksum);
	virtual bool sendPacket();
	void tick();
	SharedBuffer buffer;

public:
	nint16_t remotePort;
	nint16_t localPort;
	IPAddress remoteIP;

	UDPSocket();


	bool send();
	

	virtual bool onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data);



};







#endif