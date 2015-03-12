#include "ICMPPinger.h"

bool ICMPPinger::onICMPMessage()
{
	if (packet.icmp.type != ICMP_TYPE_ECHOREPLY)
		return false;

	if (!loadAll())
		return true; //checksum error, drop packet.

	uint8_t len = ICMP_PING_DATA_LENGTH;
	for (uint8_t* ptr = (uint8_t*)(&packet.icmp) + sizeof(ICMPHeader); len > 0; len--, ptr++)
	{
		if (*ptr != 0x79)
			return true;
	}

	onPingReply(millis() - packet.icmp.timestamp);
	return true;
}


void ICMPPinger::ping(const IPAddress& targetIP)
{
	memset(packet.icmp.dataStart(), 0x79, ICMP_PING_DATA_LENGTH);
	packet.icmp.type = ICMP_TYPE_ECHOREQUEST;
	packet.icmp.code = 0;
	packet.icmp.timestamp = millis();

	packet.icmp.checksum = 0;

	sendICMPPacket(targetIP, ICMP_PING_DATA_LENGTH);
}