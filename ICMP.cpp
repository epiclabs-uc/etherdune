#include "ICMP.h"
#include "Checksum.h"

void ICMP::onPingReply(uint16_t time){}

bool ICMP::onPacketReceived()
{
	if (!(packet.ip.protocol == IP_PROTO_ICMP_V))
	{
		return false;
	}
	
	loadAll(); //bring in the rest of the packet.

#if ENABLE_ICMP_RX_CHECKSUM

	calcICMPChecksum();
	if (packet.icmp.checksum != 0)
		return true; //checksum error, drop packet

#endif

	switch (packet.icmp.type)
	{
		case ICMP_TYPE_ECHOREQUEST_V:
		{
			packet.icmp.type = ICMP_TYPE_ECHOREPLY_V;
#if !ENABLE_ICMP_RX_CHECKSUM
			packet.icmp.checksum = 0; //checksum calculation already sets this to zero when verifying above.
#endif
			prepareIPPacket(packet.ip.sourceIP);
			calcICMPChecksum();

			sendIPPacket((uint8_t)packet.ip.totalLength.getValue());
		}break;

		case ICMP_TYPE_ECHOREPLY_V:
		{
			uint8_t len = ICMP_PING_DATA_LENGTH;
			for (uint8_t* ptr = (uint8_t*)(&packet.icmp) + sizeof(ICMPHeader); len > 0; len--, ptr++)
			{
				if (*ptr != 0x79)
					return true;
			}

			onPingReply(millis() - packet.icmp.timestamp);

		}
	}



	return true;
}

void ICMP::ping(const IPAddress& targetIP)
{
	packet.ip.totalLength.setValue(sizeof(ICMPHeader)+ ICMP_PING_DATA_LENGTH);

	packet.ip.protocol = IP_PROTO_ICMP_V;
	uint8_t len = sizeof(IPHeader) + sizeof(ICMPHeader) + ICMP_PING_DATA_LENGTH;
	packet.ip.totalLength.setValue(len);
	prepareIPPacket(targetIP);
	
	memset((uint8_t*)(&packet.icmp) + sizeof(ICMPHeader), 0x79, ICMP_PING_DATA_LENGTH);
	packet.icmp.type = ICMP_TYPE_ECHOREQUEST_V;
	packet.icmp.code = 0;
	packet.icmp.checksum = 0;
	packet.icmp.timestamp = millis();
	calcICMPChecksum();
	sendIPPacket(len);
}

void ICMP::calcICMPChecksum()
{
	packet.icmp.checksum = ~Checksum::calc(packet.ip.totalLength.getValue() - sizeof(IPHeader), (uint8_t*)&packet.icmp);
}