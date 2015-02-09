#include "NetworkService.h"
#include "ARP.h"
#include "DNS.h"

List NetworkService::activeServices;


NetworkService* NetworkService::currentService = NULL;
MACAddress NetworkService::localMAC;
IPAddress NetworkService::localIP;

static uint32_t tickTimer = NETWORK_TIMER_RESOLUTION;
EthBuffer NetworkService::chunk;


bool NetworkService::processHeader(){ return false; }
bool NetworkService::processData(uint16_t len, uint8_t* data){ return false; }
void NetworkService::tick(){}


bool NetworkService::begin(uint8_t cspin)
{
	tickTimer = millis() + NETWORK_TIMER_RESOLUTION;
	return 0!= EtherFlow::begin(cspin);
}

ARPService& NetworkService::ARP()
{
	static ARPService* arp = new ARPService();

	return *arp;
}

DNSClient& NetworkService::DNS()
{
	static DNSClient* dns = new DNSClient();

	return *dns;
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
		for (NetworkService* service = (NetworkService*)activeServices.first; service != NULL; service = (NetworkService*)service->nextItem)
		{
			if (service->processHeader())
			{
				currentService = service;
				return true;
			}
		}
		dprintln("nobody wants this packet");
		return false;
	}

	if (currentService == NULL)
	{
		dprintln("currentService should not be null");
		return false;
	}

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


	IPAddress dstIP = chunk.ip.destinationIP;
	MACAddress* dstMac = ARP().whoHas(dstIP);

	if (dstMac == NULL)
		return false;

	chunk.eth.dstMAC = *dstMac;
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