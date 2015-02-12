#ifndef __DNS__
#define __DNS__

#include <ACross.h>
#include "UDPSocket.h"
#include <FlowScanner.h>


class DNSClient;
class DNSQuery : public ListItem
{
	friend class DNSClient;

private :

	;
	
	uint8_t ptrType; //0 char*, //1 __FlashStringHelper*
	char* name;

	union
	{
		struct
		{
			uint16_t identification;
			uint8_t timer;
			uint8_t flag;
		};
		
		IPAddress resolvedIP;
	};

	



public:
	DNSQuery();
	void resolve(char* name);

	//retrieves the resolved IP address
	//returns 0 on success, 1 if resolving, 2 if timeout
	uint8_t getResponse(IPAddress& ip);
};


class DNSClient : protected UDPSocket
{
	friend class NetworkService;

private:

	List queryList;

	DNSQuery* selectedQuery;
	IPAddress resolvedIP;

	FlowScanner scanner;

	bool onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data);
	
	void tick();

public:
	DNSClient();
	uint16_t resolve(const char* name);

	void addQuery(DNSQuery& query);
	void removeQuery(DNSQuery& query);

	inline IPAddress& serverIP()
	{
		return remoteIP;
	}





};


#endif