#ifndef __DNS__
#define __DNS__

#include <ACross.h>
#include "UDPSocket.h"



class DNSClient : private UDPSocket
{
	friend class NetworkService;

private:

	uint8_t timer;

	void onReceive(uint16_t len);
	bool sendPacket();
	
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