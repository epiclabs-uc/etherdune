#ifndef __NETWORKSERVICE_H_
#define __NETWORKSERVICE_H_

#include "EtherFlow.h"
#include "List.h"

class ARPService;
class EtherFlow;
class DNSClient;

class NetworkService : private ListItem
{
	friend class EtherFlow;
	friend class DNSClient;
private:

	static List activeServices;

	static void processIncomingPacket();
	
	static void notifyOnDNSResolve(uint8_t status, uint16_t id, const IPAddress& ip);
	
protected:

	static uint8_t srcPort_L_count;

	static EthBuffer chunk;
	static ARPService ARP;


	virtual bool onPacketReceived();

	virtual void tick();
	virtual void onDNSResolve(uint8_t status, uint16_t id, const IPAddress& ip);

	static bool sendIPPacket(uint8_t headerLength);
	static void prepareIPPacket(const IPAddress& remoteIP);
	static void packetSend(uint16_t len);
	static void packetSend(uint16_t len, const byte* data);

	static bool sameLAN(IPAddress& dst);

	static void loadAll();

	NetworkService();
	~NetworkService();

public:
	static MACAddress localMAC;
	static IPAddress localIP;
	static IPAddress gatewayIP;
	static IPAddress netmask;
	static DNSClient DNS;

	static bool begin(uint8_t cspin);
	static bool isLinkUp();
	static void loop();

};

typedef NetworkService net;
 

#endif