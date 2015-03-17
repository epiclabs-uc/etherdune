// EtherFlow UDP implementation as a NetworkService
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements the UDP protocol
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

#include "UDPSocket.h"
#include "Checksum.h"

#define AC_LOGLEVEL 2
#include <ACLog.h>
ACROSS_MODULE("UDPSocket");

/// <summary>
/// Called when a datagram has arrived.
/// The data is at <c>packet.udpData</c>
/// </summary>
/// <param name="len">length of the datagram</param>
void UDPSocket::onReceive(uint16_t len) {  }


UDPSocket::UDPSocket() :sending(false)
{
#if _DEBUG
	localPort.l = random(255);
#else
	localPort.l = srcPort_L_count++;
#endif
	localPort.h = UDP_SRC_PORT_H;
}

/// <summary>
/// Sends the packet currently in the outgoing buffer
/// </summary>
/// <returns></returns>
bool UDPSocket::send()
{
	if (sending)
		return false;

	sending = true;
	UDPSocket::tick();

	return true;
}


/// <summary>
/// Fills out the UDP header and calculates the datagram checksum
/// </summary>
/// <param name="dataLength">Length of the data payload.</param>
/// <param name="dataChecksum">Checksum of the payload.</param>
void UDPSocket::prepareUDPPacket(uint16_t dataLength, uint16_t dataChecksum)
{
	packet.ip.totalLength=dataLength + sizeof(IPHeader) + sizeof(UDPHeader);
	packet.ip.protocol = IP_PROTO_UDP;
	prepareIPPacket();
	packet.udp.sourcePort = localPort;
	packet.udp.destinationPort = remotePort;

	packet.udp.dataLength = dataLength + sizeof(UDPHeader);
	packet.udp.checksum.zero();

	packet.udp.checksum.rawValue = calcUDPChecksum(dataLength, dataChecksum);

}

/// <summary>
/// Sends the packet currently in the shared buffer
/// </summary>
/// <returns></returns>
bool UDPSocket::sendPacket()
{
	uint16_t dataChecksum = 0;
	uint16_t dataLength;

	dataLength = buffer.fillTxBuffer(sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(UDPHeader),/*out*/ dataChecksum);

	prepareUDPPacket(dataLength, dataChecksum);

	if (sendIPPacket(sizeof(IPHeader) + sizeof(UDPHeader)))
	{
		buffer.flush();
		return false;
	}
	else
		return true;
}

void UDPSocket::tick()
{
	if (sending)
		sending = sendPacket();

}

bool UDPSocket::onPacketReceived()
{
	if (!(
		packet.eth.etherType == ETHTYPE_IP &&
		packet.ip.protocol == IP_PROTO_UDP &&
		localPort.rawValue == packet.udp.destinationPort.rawValue))
	{
		return false;
	}

	loadAll();

#if ENABLE_UDPTCP_RX_CHECKSUM

	if (!verifyUDPTCPChecksum())
	{
		ACWARN("UDP checksum error");
		return true;// drop packet, UDP checksum error
	}

#endif

	onReceive(packet.udp.dataLength - sizeof(UDPHeader));
	return true;
}