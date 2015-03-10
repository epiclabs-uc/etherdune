
#include "UDPSocket.h"
#include "Checksum.h"

#define AC_LOGLEVEL 2
#include <ACLog.h>
ACROSS_MODULE("UDPSocket");

void UDPSocket::onReceive(uint16_t len) {  }


UDPSocket::UDPSocket() :sending(false)
{
	localPort.l = random(255); //srcPort_L_count++;
	localPort.h = UDP_SRC_PORT_H;
}

bool UDPSocket::send()
{
	if (sending)
		return false;

	sending = true;
	UDPSocket::tick();

	return true;
}


void UDPSocket::prepareUDPPacket(uint16_t dataLength, uint16_t dataChecksum)
{
	packet.ip.totalLength.setValue(dataLength + sizeof(IPHeader) + sizeof(UDPHeader));
	packet.ip.protocol = IP_PROTO_UDP;
	prepareIPPacket();
	packet.udp.sourcePort = localPort;
	packet.udp.destinationPort = remotePort;

	packet.udp.dataLength.setValue(dataLength + sizeof(UDPHeader));
	packet.udp.checksum.zero();

	uint16_t headerChecksum = calcPseudoHeaderChecksum(IP_PROTO_UDP, dataLength + sizeof(UDPHeader));
	headerChecksum = Checksum::calc(headerChecksum, sizeof(UDPHeader), (uint8_t*)&packet.udp);

	packet.udp.checksum.rawu = calcUDPChecksum(dataLength, dataChecksum);

}

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
		packet.eth.etherType.getValue() == ETHTYPE_IP &&
		packet.ip.protocol == IP_PROTO_UDP &&
		localPort.rawu == packet.udp.destinationPort.rawu))
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

	onReceive(packet.udp.dataLength.getValue() - sizeof(UDPHeader));
	return true;
}