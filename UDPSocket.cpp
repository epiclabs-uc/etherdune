
#include "UDPSocket.h"

void UDPSocket::onReceive(uint16_t fragmentLength, uint16_t totalLength, const byte* data) {}


void UDPSocket::write(uint16_t length, uint8_t* data)
{
	buffer.write(length, data);
}
void UDPSocket::send()
{
	/*
		EtherFlow::writeBuf(TXSTART_INIT_DATA + sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(UDPHeader), len, data);


		chunk.ip.totalLength.setValue(len + sizeof(IPHeader) + sizeof(UDPHeader));
		chunk.ip.protocol = IP_PROTO_UDP_V;
		prepareIPPacket();

		chunk.udp.sourcePort = localPort;
		chunk.udp.destinationPort = remotePort;

		chunk.udp.dataLength.setValue(len + sizeof(UDPHeader));
		chunk.udp.checksum.zero();

		uint16_t checksum = calcPseudoHeaderChecksum(IP_PROTO_UDP_V, len + sizeof(UDPHeader));
		checksum = Checksum::calc(checksum, sizeof(UDPHeader), (uint8_t*)&chunk.udp);

		chunk.udp.checksum.rawu = ~Checksum::calc(checksum, len, data);

		sendIPPacket(sizeof(IPHeader) + sizeof(UDPHeader));

		return len;
*/

}

