// EtherDune ICMP automatic Echo reply class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements an ICMP Echo reply service
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

	sendIPPacket((uint8_t)packet.ip.totalLength);

	return true;

}