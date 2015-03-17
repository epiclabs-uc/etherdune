// EtherFlow ICMP Abstract base class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements common ICMP packet building routines and ICMP checksum
// Summary: Used as base class for ICMP Echo (ping)
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

#include "ICMP.h"
#include "Checksum.h"

#define AC_LOGLEVEL 2
#include <ACLog.h>
ACROSS_MODULE("ICMP");

bool ICMP::onPacketReceived()
{
	if (!(packet.ip.protocol == IP_PROTO_ICMP))
	{
		return false;
	}
	

	return onICMPMessage();

}

/// <summary>
/// Loads the entire %ICMP packet into memory.
/// </summary>
/// <returns>`true` if checksum is correct, `false` otherwise. The packet contents are nevertheless loaded into the packet buffer</returns>
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

/// <summary>
/// Calculates the %ICMP checksum and populates the checksum field in the %ICMP header.
/// If receiving, calculating the checksum sets `packet.icmp.checksum` to zero if the packet had no checksum errors.
/// </summary>
void ICMP::calcICMPChecksum()
{
	packet.icmp.checksum = ~Checksum::calc(packet.ip.totalLength - sizeof(IPHeader), (uint8_t*)&packet.icmp);
}

/// <summary>
/// Sends an %ICMP packet
/// </summary>
/// <param name="targetIP">Where to send the %ICMP packet</param>
/// <param name="dataLength">%ICMP payload length</param>
void ICMP::sendICMPPacket(const IPAddress& targetIP, uint16_t dataLength)
{
	ACASSERT(dataLength < ETHERFLOW_BUFFER_SIZE - sizeof(EthernetHeader) - sizeof(IPHeader) - sizeof(ICMPHeader), "ICMP packet too big. Actual size=%d, max size=%d", dataLength, ETHERFLOW_BUFFER_SIZE - sizeof(EthernetHeader) - sizeof(IPHeader) - sizeof(ICMPHeader));

	packet.ip.protocol = IP_PROTO_ICMP;
	dataLength += sizeof(IPHeader) + sizeof(ICMPHeader);
	packet.ip.totalLength = dataLength;
	prepareIPPacket(targetIP);

	packet.icmp.checksum = 0;
	calcICMPChecksum();
	sendIPPacket(dataLength);


}