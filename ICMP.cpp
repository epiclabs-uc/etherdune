#include "ICMP.h"
#include "Checksum.h"

void ICMP::onPingReply(uint16_t time){}

bool ICMP::onPacketReceived()
{
	if (!(chunk.ip.protocol == IP_PROTO_ICMP_V))
	{
		return false;
	}
	
	loadAll(); //bring in the rest of the packet.

#if ENABLE_ICMP_RX_CHECKSUM

	calcICMPChecksum();
	if (chunk.icmp.checksum != 0)
		return true; //checksum error, drop packet

#endif

	switch (chunk.icmp.type)
	{
		case ICMP_TYPE_ECHOREQUEST_V:
		{
			chunk.icmp.type = ICMP_TYPE_ECHOREPLY_V;
#if !ENABLE_ICMP_RX_CHECKSUM
			chunk.icmp.checksum = 0; //checksum calculation already sets this to zero when verifying above.
#endif
			prepareIPPacket(chunk.ip.sourceIP);
			calcICMPChecksum();

			sendIPPacket((uint8_t)chunk.ip.totalLength.getValue());
		}break;

		case ICMP_TYPE_ECHOREPLY_V:
		{
			uint8_t len = ICMP_PING_DATA_LENGTH;
			for (uint8_t* ptr = (uint8_t*)(&chunk.icmp) + sizeof(ICMPHeader); len > 0; len--, ptr++)
			{
				if (*ptr != 0x79)
					return true;
			}

			onPingReply(millis() - chunk.icmp.timestamp);

		}
	}



	return true;
}

void ICMP::ping(const IPAddress& targetIP)
{
	chunk.ip.totalLength.setValue(sizeof(ICMPHeader)+ ICMP_PING_DATA_LENGTH);

	chunk.ip.protocol = IP_PROTO_ICMP_V;
	uint8_t len = sizeof(IPHeader) + sizeof(ICMPHeader) + ICMP_PING_DATA_LENGTH;
	chunk.ip.totalLength.setValue(len);
	prepareIPPacket(targetIP);
	
	memset((uint8_t*)(&chunk.icmp) + sizeof(ICMPHeader), 0x79, ICMP_PING_DATA_LENGTH);
	chunk.icmp.type = ICMP_TYPE_ECHOREQUEST_V;
	chunk.icmp.code = 0;
	chunk.icmp.checksum = 0;
	chunk.icmp.timestamp = millis();
	calcICMPChecksum();
	sendIPPacket(len);
}

void ICMP::calcICMPChecksum()
{
	chunk.icmp.checksum = ~Checksum::calc(chunk.ip.totalLength.getValue() - sizeof(IPHeader), (uint8_t*)&chunk.icmp);
}