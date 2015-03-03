
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
	chunk.ip.totalLength.setValue(dataLength + sizeof(IPHeader) + sizeof(UDPHeader));
	chunk.ip.protocol = IP_PROTO_UDP_V;
	prepareIPPacket();
	chunk.udp.sourcePort = localPort;
	chunk.udp.destinationPort = remotePort;

	chunk.udp.dataLength.setValue(dataLength + sizeof(UDPHeader));
	chunk.udp.checksum.zero();

	uint16_t headerChecksum = calcPseudoHeaderChecksum(IP_PROTO_UDP_V, dataLength + sizeof(UDPHeader));
	headerChecksum = Checksum::calc(headerChecksum, sizeof(UDPHeader), (uint8_t*)&chunk.udp);

	chunk.udp.checksum.rawu = calcUDPChecksum(dataLength, dataChecksum);

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
		chunk.eth.etherType.getValue() == ETHTYPE_IP &&
		chunk.ip.protocol == IP_PROTO_UDP_V &&
		localPort.rawu == chunk.udp.destinationPort.rawu))
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

	onReceive(chunk.udp.dataLength.getValue() - sizeof(UDPHeader));
	return true;
}