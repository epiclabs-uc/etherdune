#ifndef __DNS__
#define __DNS__

#include <ACross.h>
#include "UDPSocket.h"



class DNSClient : private UDPSocket
{
	friend class NetworkService;

private:

	IPAddress resolvedIP;
	uint8_t timer;
	uint16_t identification;
	uint16_t dataLength;
	uint16_t dataPos;
	

	bool onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data);
	bool sendPacket();
	void nextQuery();
	
	void tick();

public:
	DNSClient();
	uint16_t resolve(const char* name);

	inline IPAddress& serverIP()
	{
		return remoteIP;
	}






};


#endif