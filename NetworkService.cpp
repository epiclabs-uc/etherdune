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
DNSClient NetworkService::DNS;

MACAddress NetworkService::localMAC;
IPAddress NetworkService::localIP;
IPAddress NetworkService::gatewayIP;
IPAddress NetworkService::netmask;

uint8_t NetworkService::srcPort_L_count = 0;




static uint32_t tickTimer = NETWORK_TIMER_RESOLUTION;
EthBuffer NetworkService::chunk;


bool NetworkService::onPacketReceived(){ return false; }
void NetworkService::tick(){}
void NetworkService::onDNSResolve(uint8_t status, uint16_t id, const IPAddress& ip) {}


bool NetworkService::begin(uint8_t cspin)
{
	tickTimer = millis() + NETWORK_TIMER_RESOLUTION;
	return 0!= EtherFlow::begin(cspin);
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
	ACDEBUG("Incoming packet etherType=%x", chunk.eth.etherType.getValue());

#if ENABLE_IP_RX_CHECKSUM || ENABLE_UDPTCP_RX_CHECKSUM

		if (chunk.eth.etherType.getValue() == ETHTYPE_IP)
		{
			uint16_t sum = ~Checksum::calc(sizeof(IPHeader), (uint8_t*)&chunk.ip);
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
	EtherFlow::loadSample();

	if ((int32_t)(millis() - tickTimer) >= 0)
	{

		for (NetworkService* service = (NetworkService*)activeServices.first; service != NULL; service = (NetworkService*)service->nextItem)
			service->tick();

		tickTimer = millis() + NETWORK_TIMER_RESOLUTION;
	}
}

bool NetworkService::sendIPPacket(uint8_t headerLength)
{

	if (chunk.ip.destinationIP.b[3] == 255) //(cheap hack to detect if it is an IP-layer broadcast)
	{
		//LAN broadcast then.
		memset(&chunk.eth.dstMAC, 0xFF, sizeof(MACAddress));
	}
	else
	{
		IPAddress dstIP = sameLAN(chunk.ip.destinationIP) ? chunk.ip.destinationIP : gatewayIP;

		MACAddress* dstMac = ARP.whoHas(dstIP);

		if (dstMac == NULL)
			return false;

		chunk.eth.dstMAC = *dstMac;
	}

	chunk.eth.srcMAC = localMAC;
	chunk.eth.etherType.setValue(ETHTYPE_IP);

	EtherFlow::writeBuf(TXSTART_INIT_DATA, sizeof(EthernetHeader) + headerLength, chunk.raw);
	EtherFlow::packetSend(sizeof(EthernetHeader) + chunk.ip.totalLength.getValue());

	return true;
}

bool NetworkService::isLinkUp()
{
	return EtherFlow::isLinkUp();
}

void NetworkService::packetSend(uint16_t len)
{
	EtherFlow::packetSend(len);
}
void NetworkService::packetSend(uint16_t len, const byte* data)
{
	EtherFlow::packetSend(len, data);
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
	chunk.ip.version = 4;
	chunk.ip.IHL = 0x05; //20 bytes
	chunk.ip.raw[1] = 0x00; //DSCP/ECN=0;
	chunk.ip.identification.setValue(0);
	chunk.ip.flags = 0;
	chunk.ip.fragmentOffset = 0;
	chunk.ip.checksum.setValue(0);
	chunk.ip.sourceIP = localIP;
	chunk.ip.destinationIP = remoteIP;
	chunk.ip.TTL = 255;
	chunk.ip.checksum.rawu = ~Checksum::calc(sizeof(IPHeader), (uint8_t*)&chunk.ip);
}

void NetworkService::loadAll()
{
#if (ETHERFLOW_SAMPLE_SIZE < ETHERFLOW_BUFFER_SIZE)
	EtherFlow::loadAll();
#endif
}
