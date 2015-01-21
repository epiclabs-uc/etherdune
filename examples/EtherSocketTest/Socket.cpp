// 
// 
// 

#include "Socket.h"

uint8_t Socket::srcPort_L_count = 0;


void Socket::onClose() {}
void Socket::onConnect() {}

Socket::Socket()
{
	
	state = 0;
	sequenceNumber = 0;
	EtherSocket::registerSocket(*this);
}

Socket::~Socket()
{
	EtherSocket::unregisterSocket(*this);
}


void Socket::connect()
{

	srcPort_L = srcPort_L_count++;
	retries = 0;
	sendSYN();



}

void Socket::sendSYN()
{
	Serial.println("sendSYN()");
	retries++;
	if (retries >= MAX_TCP_CONNECT_RETRIES)
	{
		onClose();
		state = SCK_STATE_CLOSED;
		return;
	}

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
	EtherSocket::chunk.ip.destinationIP = remoteAddress;
	EtherSocket::chunk.ip.TTL = 255;
	EtherSocket::chunk.ip.checksum.setValue(~EtherSocket::checksum(0, (uint8_t*)&EtherSocket::chunk.ip, sizeof(IPHeader)));


	EtherSocket::chunk.tcp.sourcePort.h = TCP_SRC_PORT_H;
	EtherSocket::chunk.tcp.sourcePort.l = srcPort_L;
	EtherSocket::chunk.tcp.destinationPort = remotePort;
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

	state = SCK_STATE_SYN_SENT;

	EtherSocket::sendIPPacket();
}

void Socket::tick()
{
	
	
	switch (state)
	{
		case SCK_STATE_SYN_SENT:
		{
			sendSYN();
			break;
		}
	}

}


bool Socket::processSegment(bool isHeader)
{
	Serial.println("segment received");
	Serial.print("SYN="); Serial.println(EtherSocket::chunk.tcp.SYN);
	Serial.print("ACK="); Serial.println(EtherSocket::chunk.tcp.ACK);

	if (isHeader)
	{
		uint32_t ackNum = EtherSocket::chunk.tcp.acknowledgementNumber.getValue();
		if ((int32_t)(sequenceNumber - ackNum) < 0)
		{
			sequenceNumber = ackNum;
		}
		else
		{
			Serial.println("dropped duplicate packet");
			return false;
		}

		switch (state)
		{
			case SCK_STATE_SYN_SENT:
			{
				if (EtherSocket::chunk.tcp.SYN && EtherSocket::chunk.tcp.ACK)
				{
					state = SCK_STATE_ESTABLISHED;
					onConnect();
					//now, send SYN ACK
				}


			}

			default:
				break;
		}
	}


	return true;

}

