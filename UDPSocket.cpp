
#include "UDPSocket.h"
#include "Checksum.h"

bool UDPSocket::onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data) { return false; }


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
	prepareIPPacket(remoteIP);
	chunk.udp.sourcePort = localPort;
	chunk.udp.destinationPort = remotePort;

	chunk.udp.dataLength.setValue(dataLength + sizeof(UDPHeader));
	chunk.udp.checksum.zero();

	uint16_t headerChecksum = calcPseudoHeaderChecksum(IP_PROTO_UDP_V, dataLength + sizeof(UDPHeader));
	headerChecksum = Checksum::calc(headerChecksum, sizeof(UDPHeader), (uint8_t*)&chunk.udp);

	chunk.udp.checksum.rawu = ~Checksum::add(headerChecksum, dataChecksum);

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

bool UDPSocket::processHeader()
{
	if (!(
		chunk.eth.etherType.getValue() == ETHTYPE_IP &&
		chunk.ip.protocol == IP_PROTO_UDP_V &&
		localPort.rawu == chunk.udp.destinationPort.rawu))
	{
		return false;
	}

	
	uint16_t datagramLength = chunk.udp.dataLength.getValue();
	uint16_t fragmentLength = min(datagramLength, sizeof(EthBuffer) - sizeof(EthernetHeader) - sizeof(IPHeader) - sizeof(UDPHeader));

	return onReceive(fragmentLength, datagramLength, chunk.raw + sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(UDPHeader));

}



bool UDPSocket::processData(uint16_t len, uint8_t* data)
{
	return onReceive(len, 0, data);
}
