#ifndef __DNS__
#define __DNS__

#include <ACross.h>
#include "UDPSocket.h"
#include <FlowScanner.h>

class DNSClient : protected UDPSocket
{
	friend class NetworkService;

private:
	uint16_t identification;
	uint16_t receivedId;
	uint8_t timer;

	FlowScanner scanner;

	void onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data);
	
	void tick();

public:
	DNSClient();
	bool resolve(const char* name);

	inline IPAddress& serverIP()
	{
		return remoteIP;
	}

	IPAddress resolvedIP;




};


#endif