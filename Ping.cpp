#include "Ping.h"
#include "Checksum.h"



bool Ping::onPacketReceived()
{
	if (!(chunk.ip.protocol == IP_PROTO_ICMP_V && chunk.icmp.type == ICMP_TYPE_ECHOREQUEST_V))
	{
		return false;
	}
	loadAll(); //bring in the rest of the packet.

	chunk.icmp.type = ICMP_TYPE_ECHOREPLY_V;
	chunk.icmp.checksum = 0;
	prepareIPPacket(chunk.ip.sourceIP);
	chunk.icmp.checksum = ~Checksum::calc(chunk.ip.totalLength.getValue() - sizeof(IPHeader),(uint8_t*) &chunk.icmp);

	sendIPPacket((uint8_t)chunk.ip.totalLength.getValue());


	return true;
}