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


NetworkService* NetworkService::currentService = NULL;
MACAddress NetworkService::localMAC;
IPAddress NetworkService::localIP;
IPAddress NetworkService::gatewayIP;
IPAddress NetworkService::netmask;

uint8_t NetworkService::srcPort_L_count = 0;




static uint32_t tickTimer = NETWORK_TIMER_RESOLUTION;
EthBuffer NetworkService::chunk;


bool NetworkService::processHeader(){ return false; }
bool NetworkService::processData(uint16_t len, uint8_t* data){ return false; }
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
bool NetworkService::processChunk(bool isHeader, uint16_t length)
{
	if (isHeader)
	{

#if ENABLE_IP_RX_CHECKSUM || ENABLE_UDPTCP_RX_CHECKSUM

		if (chunk.eth.etherType.getValue() == ETHTYPE_IP)
		{
			uint16_t sum = ~Checksum::calc(sizeof(IPHeader), (uint8_t*)&chunk.ip);
			if (0 != sum)
			{
				ACWARN("IP Header checksum error");
				return false; // drop packet, IP Header checksum error
			}

#if ENABLE_UDPTCP_RX_CHECKSUM

			if (!verifyUDPTCPChecksum())
			{
				ACWARN("TCP/UDP checksum error");
				return false;// drop packet, TCP checksum error
			}

#endif



		}
#endif

		for (NetworkService* service = (NetworkService*)activeServices.first; service != NULL; service = (NetworkService*)service->nextItem)
		{
			if (service->processHeader())
			{
				currentService = service;
				return true;
			}
		}
		ACTRACE("nobody wants this packet");
		return false;
	}

	ACBREAK(currentService != NULL, "currentService is NULL");

	return currentService->processData(length, chunk.raw);


}

void NetworkService::loop()
{
	EtherFlow::loop();

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

uint16_t NetworkService::calcPseudoHeaderChecksum(uint8_t protocol, uint16_t length)
{
	nint32_t pseudo;
	pseudo.h.h = 0;
	pseudo.h.l = protocol;
	pseudo.l.setValue(length);

	uint16_t sum = Checksum::calc(sizeof(IPAddress) * 2, (uint8_t*)&chunk.ip.sourceIP);
	return Checksum::calc(sum, sizeof(pseudo), (uint8_t*)&pseudo);
}

uint16_t NetworkService::calcTCPChecksum(bool options, uint16_t dataLength, uint16_t dataChecksum)
{
	uint8_t headerLength = options ? sizeof(TCPOptions) + sizeof(TCPHeader) : sizeof(TCPHeader);
	uint16_t sum = calcPseudoHeaderChecksum(IP_PROTO_TCP_V, dataLength + headerLength);
	sum = Checksum::calc(sum, headerLength, (uint8_t*)&chunk.tcp);
	sum = Checksum::add(sum, dataChecksum);
	return ~sum;
}

uint16_t NetworkService::calcUDPChecksum(uint16_t dataLength, uint16_t dataChecksum)
{
	uint16_t headerChecksum = calcPseudoHeaderChecksum(IP_PROTO_UDP_V, dataLength + sizeof(UDPHeader));
	headerChecksum = Checksum::calc(headerChecksum, sizeof(UDPHeader), (uint8_t*)&chunk.udp);

	return ~Checksum::add(headerChecksum, dataChecksum);
}


bool NetworkService::verifyUDPTCPChecksum()
{
#if ENABLE_UDPTCP_RX_CHECKSUM
	uint8_t headerLength;
	uint16_t dataOffset;
	switch (chunk.ip.protocol)
	{
		case IP_PROTO_TCP_V:
		{
			headerLength = sizeof(IPHeader) + sizeof(TCPHeader);
			dataOffset = sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(TCPHeader);
		}break;
		case IP_PROTO_UDP_V:
		{
			headerLength = sizeof(IPHeader) + sizeof(UDPHeader);
			dataOffset = sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(UDPHeader);
		}break;
		default:
			return true;
	}

	uint16_t dataChecksum;

	uint16_t totalLength = chunk.ip.totalLength.getValue();
	uint16_t dataLength = totalLength - headerLength;

	if (totalLength <= sizeof(EthBuffer) - sizeof(EthernetHeader))
	{
		//calculate via software since the entire packet fits in RAM
		dataChecksum = Checksum::calc(dataLength, chunk.raw + dataOffset);
	}
	else
	{
#if ENABLE_HW_CHECKSUM
		//calculate via hardware, no other choice.
		//warning: this may cause packet loss
		// see ENC28J60 Silicon errata, issue 17.
		nint16_t chk;
		chk.rawu = ~EtherFlow::hardwareChecksumRxOffset(dataOffset, dataLength);
		dataChecksum = chk.getValue();
#else
		return true;
#endif
	}

	uint16_t sum;
	if (chunk.ip.protocol == IP_PROTO_TCP_V)
		sum = calcTCPChecksum(false,dataLength, dataChecksum);
	else
		sum = calcUDPChecksum( dataLength, dataChecksum);
	
	return 0 == sum;


#else
	return true;
#endif



}