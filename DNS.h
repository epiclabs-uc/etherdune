#ifndef __DNS__
#define __DNS__

#include <ACross/ACross.h>
#include "UDPSocket.h"
#include <FlowScanner/FlowScanner.h>

class DNSClient : public UDPSocket
{
	friend class NetworkService;

private:
	uint16_t identification;
	uint16_t receivedId;
	uint8_t timer;

	FlowScanner scanner;

	void onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data);
	DNSClient();
	void tick();

public:

	bool resolve(const char* name);
	void setDNSAddress(const IPAddress& dnsServerIP);

	IPAddress resolvedIP;




};


#endif