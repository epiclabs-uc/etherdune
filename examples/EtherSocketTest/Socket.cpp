// 
// 
// 

#include "Socket.h"

uint8_t Socket::srcPort_L_count = 0;

Socket::Socket(SocketCallback eventHandlerCallback)
{
	eventHandler = eventHandlerCallback;
	state = 0;
	sequenceNumber = 0;
}



void Socket::connect(IPAddress& ip, uint16_t port)
{
	dstAddr = ip;
	dstPort.setValue(port);
	srcPort_L = srcPort_L_count++;

	EtherSocket::chunk.ip.version = 4;
	EtherSocket::chunk.ip.IHL = 0x05; //20 bytes
	EtherSocket::chunk.ip.raw[1] = 0x00; //DSCP/ECN=0;
	EtherSocket::chunk.ip.totalLength.setValue(sizeof(IPHeader) + sizeof(TCPHeader));
	EtherSocket::chunk.ip.identification.setValue(0);
	EtherSocket::chunk.ip.flags = 0;
	EtherSocket::chunk.ip.fragmentOffset = 0;
	EtherSocket::chunk.ip.protocol = IP_PROTO_TCP_V;
	EtherSocket::chunk.ip.checksum.setValue(0);
	EtherSocket::chunk.ip.sourceIP = EtherSocket::localIP;
	EtherSocket::chunk.ip.destinationIP = ip;
	EtherSocket::chunk.ip.TTL = 255;
	EtherSocket::chunk.ip.checksum.setValue(~ EtherSocket::checksum(0, (uint8_t*)&EtherSocket::chunk.ip, sizeof(IPHeader)));


	EtherSocket::chunk.tcp.sourcePort.h = TCP_SRC_PORT_H;
	EtherSocket::chunk.tcp.sourcePort.l = srcPort_L;
	EtherSocket::chunk.tcp.destinationPort = dstPort;
	EtherSocket::chunk.tcp.sequenceNumber.setValue(sequenceNumber);
	EtherSocket::chunk.tcp.flags = 0x00;
	EtherSocket::chunk.tcp.SYN = 1;
	EtherSocket::chunk.tcp.windowSize.setValue(512);
	EtherSocket::chunk.tcp.acknowledgementNumber.zero();

	EtherSocket::chunk.tcp.checksum.zero();
	EtherSocket::chunk.tcp.options.option1 = 0x02;
	EtherSocket::chunk.tcp.options.option1_length = 0x04;
	EtherSocket::chunk.tcp.options.option1_value.setValue(TCP_MAXIMUM_SEGMENT_SIZE);
	EtherSocket::chunk.tcp.dataOffset = 5 + 1; // 5 words normal length + 1 word because of options.

	
	uint16_t sum;
	sum = EtherSocket::checksum(0, (uint8_t*)&EtherSocket::chunk.ip.sourceIP, sizeof(IPAddress) * 2);
	nint32_t pseudo;
	pseudo.h.h = 0;
	pseudo.h.l = IP_PROTO_TCP_V;
	pseudo.l.setValue(sizeof(TCPHeader));




	
	sum = EtherSocket::checksum(sum, (uint8_t*)&pseudo, sizeof(pseudo));



	sum = EtherSocket::checksum(sum, (uint8_t*)&EtherSocket::chunk.tcp, sizeof(TCPHeader));
	EtherSocket::chunk.tcp.checksum.setValue(~sum);

	EtherSocket::sendIPPacket();

}