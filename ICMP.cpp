#include "ICMP.h"
#include "Checksum.h"

bool ICMP::onPacketReceived()
{
	if (!(packet.ip.protocol == IP_PROTO_ICMP))
	{
		return false;
	}
	

	return onICMPMessage();

}

bool ICMP::loadAll()
{
	NetworkService::loadAll(); //bring in the rest of the packet.

#if ENABLE_ICMP_RX_CHECKSUM

	calcICMPChecksum();
	if (packet.icmp.checksum != 0)
		return false; //checksum error, drop packet

#endif

	return true;
}

void ICMP::calcICMPChecksum()
{
	packet.icmp.checksum = ~Checksum::calc(packet.ip.totalLength.getValue() - sizeof(IPHeader), (uint8_t*)&packet.icmp);
}

void ICMP::sendICMPPacket(const IPAddress& targetIP, uint16_t dataLength)
{
	packet.ip.protocol = IP_PROTO_ICMP;
	dataLength += sizeof(IPHeader) + sizeof(ICMPHeader);
	packet.ip.totalLength.setValue(dataLength);
	prepareIPPacket(targetIP);

	packet.icmp.checksum = 0;
	calcICMPChecksum();
	sendIPPacket(dataLength);


}