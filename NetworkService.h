#ifndef __NETWORKSERVICE_H_
#define __NETWORKSERVICE_H_

#include "EtherFlow.h"
#include "List.h"

class ARPService;
class EtherFlow;

class NetworkService : private ListItem
{
	friend class EtherFlow;
private:

	static List activeServices;
	static NetworkService* currentService;

	static bool processChunk(bool isHeader, uint16_t length);	
	
	
	
protected:

	static ARPService& ARP();

	static EthBuffer chunk;

	virtual bool processHeader();
	virtual bool processData(uint16_t len, uint8_t* data);

	virtual void tick();

	static void sendIPPacket(uint8_t headerLength);
	static void packetSend(uint16_t len);
	static void packetSend(uint16_t len, const byte* data);

	NetworkService();
	~NetworkService();

public:
	static MACAddress localMAC;
	static IPAddress localIP;

	static bool begin(uint8_t cspin);
	static bool isLinkUp();
	static void loop();

};

typedef NetworkService net;



#endif