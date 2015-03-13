#ifndef __NETWORKSERVICE_H_
#define __NETWORKSERVICE_H_

#include "ENC28J60.h"
#include "List.h"

class ARPService;
class DNSClient;


class NetworkService : private ListItem, public ENC28J60
{
	friend class DNSClient;
	friend class ENC28J60;

private:

	static List activeServices;

	static void processIncomingPacket();
	static void notifyOnDNSResolve(uint8_t status, uint16_t identification, const IPAddress& ip);
	
protected:

	static EthBuffer packet;
	static ARPService ARP;

	NetworkService();
	~NetworkService();

	static bool sendIPPacket(uint16_t headerLength);
	static void prepareIPPacket(const IPAddress& remoteIP);
	static bool sameLAN(IPAddress& dst);

	virtual bool onPacketReceived();
	virtual void tick();
	virtual void onDNSResolve(uint8_t status, uint16_t identification, const IPAddress& ip);

public:
	static MACAddress localMAC;
	static IPAddress localIP;
	static IPAddress gatewayIP;
	static IPAddress netmask;
	static IPAddress dnsIP;
	

	static bool begin(uint8_t cspin);
	static void loop();
	static DNSClient& DNS();

};

typedef NetworkService net;
 

#endif