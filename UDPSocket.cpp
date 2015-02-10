
#include "UDPSocket.h"
#include "Checksum.h"

void UDPSocket::onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data) {}


UDPSocket::UDPSocket() :sending(false)
{
}

void UDPSocket::write(uint16_t length, uint8_t* data)
{
	buffer.write(length, data);
}
bool UDPSocket::send()
{
	if (sending)
		return false;

	sending = true;
	tick();

	return true;
}

void UDPSocket::tick()
{
	if (sending)
	{
		uint16_t dataChecksum = 0;
		uint16_t dataLength;

		dataLength = buffer.fillTxBuffer(sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(UDPHeader),/*out*/ dataChecksum);

		chunk.ip.totalLength.setValue(dataLength + sizeof(IPHeader) + sizeof(UDPHeader));
		chunk.ip.protocol = IP_PROTO_UDP_V;
		prepareIPPacket();
		chunk.udp.sourcePort = localPort;
		chunk.udp.destinationPort = remotePort;

		chunk.udp.dataLength.setValue(dataLength + sizeof(UDPHeader));
		chunk.udp.checksum.zero();

		uint16_t headerChecksum = calcPseudoHeaderChecksum(IP_PROTO_UDP_V, dataLength + sizeof(UDPHeader));
		headerChecksum = Checksum::calc(headerChecksum, sizeof(UDPHeader), (uint8_t*)&chunk.udp);

		chunk.udp.checksum.rawu = ~Checksum::add(headerChecksum, dataChecksum);

		if (sendIPPacket(sizeof(IPHeader) + sizeof(UDPHeader)))
		{
			buffer.flush();
			sending = false;
		}
	}

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

	onReceive(fragmentLength, datagramLength, chunk.raw + sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(UDPHeader));

	return true;
}



bool UDPSocket::processData(uint16_t len, uint8_t* data)
{
	onReceive(len, 0, data);
	return true;
}
