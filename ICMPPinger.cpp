// EtherDune ICMP Pinger class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements an ICMP Echo request service
//
// Copyright (c) 2015 All Rights Reserved, http://friendev.com
//
// This source is subject to the GPLv2 license.
// Please see the License.txt file for more information.
// All other rights reserved.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.

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


/// <summary>
/// Send an %ICMP Echo request (ping) to the specified target IP address.
/// You can specify the length of the test payload by editing the \ref ICMP_PING_DATA_LENGTH configuration constant.
/// See \ref ICMPConfig for more information
/// </summary>
/// <param name="targetIP">The target IP address</param>
void ICMPPinger::ping(const IPAddress& targetIP)
{
	memset(packet.icmp.dataStart(), 0x79, ICMP_PING_DATA_LENGTH);
	packet.icmp.type = ICMP_TYPE_ECHOREQUEST;
	packet.icmp.code = 0;
	packet.icmp.timestamp = millis();

	packet.icmp.checksum = 0;

	sendICMPPacket(targetIP, ICMP_PING_DATA_LENGTH);
}