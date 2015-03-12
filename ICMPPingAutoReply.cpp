#include "ICMPPingAutoReply.h"



bool ICMPPingAutoReply::onICMPMessage()
{
	if (packet.icmp.type != ICMP_TYPE_ECHOREQUEST)
		return false;

	if (!loadAll())
		return true; //checksum error, drop packet.

	packet.icmp.type = ICMP_TYPE_ECHOREPLY;

#if !ENABLE_ICMP_RX_CHECKSUM
	packet.icmp.checksum = 0; //checksum calculation already sets this to zero in ICMP::loadAll().
#endif
	prepareIPPacket(packet.ip.sourceIP);
	calcICMPChecksum();

	sendIPPacket((uint8_t)packet.ip.totalLength.getValue());

	return true;

}