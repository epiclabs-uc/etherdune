#ifndef __DNS__
#define __DNS__

#include <ACross.h>
#include "UDPSocket.h"
#include <FlowScanner.h>


class DNSClient : protected UDPSocket
{
	friend class NetworkService;

private:

	IPAddress resolvedIP;
	uint8_t timer;
	uint16_t identification;

	FlowScanner scanner;

	bool onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data);
	bool sendPacket();
	
	void tick();

public:
	DNSClient();
	uint16_t resolve(const char* name);

	inline IPAddress& serverIP()
	{
		return remoteIP;
	}

	virtual void onResolve(uint16_t id, const IPAddress& ip);





};


#endif