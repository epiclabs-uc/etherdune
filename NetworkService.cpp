#include "NetworkService.h"
#include "ARP.h"
#include "DNS.h"
#include "Checksum.h"

#define AC_LOGLEVEL 2
#include <ACLog.h>
ACROSS_MODULE("NetworkService");


List NetworkService::activeServices;
List SharedBuffer::bufferList;

ARPService NetworkService::ARP;

MACAddress NetworkService::localMAC;
IPAddress NetworkService::localIP = { 0, 0, 0, 0 };
IPAddress NetworkService::gatewayIP = { 0, 0, 0, 0 };
IPAddress NetworkService::netmask = { 0, 0, 0, 0 };
IPAddress NetworkService::dnsIP = { 0, 0, 0, 0 };





static uint32_t tickTimer = NETWORK_TIMER_RESOLUTION;
EthBuffer NetworkService::packet;


bool NetworkService::onPacketReceived(){ return false; }
void NetworkService::tick(){}
void NetworkService::onDNSResolve(uint8_t status, uint16_t identification, const IPAddress& ip) {}


bool NetworkService::begin(uint8_t cspin)
{
	tickTimer = millis() + NETWORK_TIMER_RESOLUTION;
	return 0!= ENC28J60::begin(cspin);
}


NetworkService::NetworkService()
{
	activeServices.add(this);
}
NetworkService::~NetworkService()
{
	activeServices.remove(this);
}
void NetworkService::processIncomingPacket()
{
	ACDEBUG("Incoming packet etherType=%x", packet.eth.etherType.getValue());

#if ENABLE_IP_RX_CHECKSUM || ENABLE_UDPTCP_RX_CHECKSUM

		if (packet.eth.etherType.getValue() == ETHTYPE_IP)
		{
			uint16_t sum = ~Checksum::calc(sizeof(IPHeader), (uint8_t*)&packet.ip);
			if (0 != sum)
			{
				ACWARN("IP Header checksum error");
				return ; // drop packet, IP Header checksum error
			}
		}
#endif

		for (NetworkService* service = (NetworkService*)activeServices.first; service != NULL; service = (NetworkService*)service->nextItem)
		{
			if (service->onPacketReceived())
				return;

		}
		ACTRACE("nobody wants this packet");
		return;
	
}

void NetworkService::loop()
{
	ENC28J60::loadSample();

	if ((int32_t)(millis() - tickTimer) >= 0)
	{

		for (NetworkService* service = (NetworkService*)activeServices.first; service != NULL; service = (NetworkService*)service->nextItem)
			service->tick();

		tickTimer = millis() + NETWORK_TIMER_RESOLUTION;
	}
}

bool NetworkService::sendIPPacket(uint16_t headerLength)
{

	if (packet.ip.destinationIP.b[3] == 255) //(cheap hack to detect if it is an IP-layer broadcast)
	{
		//LAN broadcast then.
		memset(&packet.eth.dstMAC, 0xFF, sizeof(MACAddress));
	}
	else
	{
		IPAddress dstIP = sameLAN(packet.ip.destinationIP) ? packet.ip.destinationIP : gatewayIP;

		MACAddress* dstMac = ARP.whoHas(dstIP);

		if (dstMac == NULL)
			return false;

		packet.eth.dstMAC = *dstMac;
	}

	packet.eth.srcMAC = localMAC;
	packet.eth.etherType.setValue(ETHTYPE_IP);

	ENC28J60::writeBuf(TXSTART_INIT_DATA, sizeof(EthernetHeader) + headerLength, packet.raw);
	ENC28J60::packetSend(sizeof(EthernetHeader) + packet.ip.totalLength.getValue());

	return true;
}


bool NetworkService::sameLAN(IPAddress& dst)
{
	if (localIP.b[0] == 0 || dst.b[0] == 0) 
		return false;

	for (int i = 0; i < 4; i++)
		if ((localIP.b[i] & netmask.b[i]) != (dst.b[i] & netmask.b[i]))
			return false;

	return true;
}

void NetworkService::notifyOnDNSResolve(uint8_t status, uint16_t id, const IPAddress& ip)
{
	for (NetworkService* service = (NetworkService*)activeServices.first; service != NULL; service = (NetworkService*)service->nextItem)
	{
		service->onDNSResolve(status, id, ip);
	}
}



void NetworkService::prepareIPPacket(const IPAddress& remoteIP)
{
	packet.ip.version = 4;
	packet.ip.IHL = 0x05; //20 bytes
	packet.ip.raw[1] = 0x00; //DSCP/ECN=0;
	packet.ip.identification.setValue(0);
	packet.ip.flags = 0;
	packet.ip.fragmentOffset = 0;
	packet.ip.destinationIP = remoteIP;
	packet.ip.sourceIP = localIP;
	packet.ip.TTL = 255;
	packet.ip.checksum.setValue(0);
	packet.ip.checksum.rawu = ~Checksum::calc(sizeof(IPHeader), (uint8_t*)&packet.ip);
}

DNSClient& NetworkService::DNS()
{
	static DNSClient dns;
	return dns;
}

