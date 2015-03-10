// 
// 
// 

#include "Socket.h"
#include "Checksum.h"

#define AC_LOGLEVEL 2
#include <ACLog.h>
ACROSS_MODULE("Socket");

uint8_t Socket::srcPort_L_count = 0;



void Socket::prepareIPPacket()
{
	packet.ip.version = 4;
	packet.ip.IHL = 0x05; //20 bytes
	packet.ip.raw[1] = 0x00; //DSCP/ECN=0;
	packet.ip.identification.setValue(0);
	packet.ip.flags = 0;
	packet.ip.fragmentOffset = 0;
	packet.ip.checksum.setValue(0);
	packet.ip.sourceIP = localIP;
	packet.ip.destinationIP = remoteIP;
	packet.ip.TTL = 255;
	packet.ip.checksum.rawu = ~Checksum::calc(sizeof(IPHeader), (uint8_t*)&packet.ip);
}




uint16_t Socket::write(uint16_t len, const byte* data)
{
	ACTRACE("write %d bytes. Dump:",len )
	AC_TRACE(Serial.write(data, len));
	AC_TRACE(Serial.println());

	return buffer.write(len, data);
}

uint16_t Socket::write(const String& s)
{
	return write(s.length(), (uint8_t*)s.c_str());
}

uint16_t Socket::write(const __FlashStringHelper* pattern, ...)
{
	char c;
	char buf[16];
	uint8_t i = 0;
	va_list args;
	va_start(args, pattern);
	PGM_P p = (PGM_P)pattern;
	uint16_t bytes = 0;

	for (;;)
	{
		c = (char)pgm_read_byte(p++);

		if (c == '%')
		{
			c = (char)pgm_read_byte(p);
			if (c != '%')
			{
				bytes += write(i, (uint8_t*)buf);
				bytes += write(*va_arg(args, String*));
				i = 0;
				continue;
			}
			p++;
		}

		if (c == 0)
		{
			write(i, (uint8_t*)buf);
			va_end(args);
			return bytes;
		}

		buf[i] = c;
		bytes++;
		i++;
		if (i == sizeof(buf))
		{
			write(i, (uint8_t*)buf);
			i = 0;

		}


	}

}

uint16_t Socket::calcPseudoHeaderChecksum(uint8_t protocol, uint16_t length)
{
	nint32_t pseudo;
	pseudo.h.h = 0;
	pseudo.h.l = protocol;
	pseudo.l.setValue(length);

	uint16_t sum = Checksum::calc(sizeof(IPAddress) * 2, (uint8_t*)&packet.ip.sourceIP);
	return Checksum::calc(sum, sizeof(pseudo), (uint8_t*)&pseudo);
}

uint16_t Socket::calcTCPChecksum(bool options, uint16_t dataLength, uint16_t dataChecksum)
{
	uint8_t headerLength = options ? sizeof(TCPOptions) + sizeof(TCPHeader) : sizeof(TCPHeader);
	uint16_t sum = calcPseudoHeaderChecksum(IP_PROTO_TCP_V, dataLength + headerLength);
	sum = Checksum::calc(sum, headerLength, (uint8_t*)&packet.tcp);
	sum = Checksum::add(sum, dataChecksum);
	return ~sum;
}

uint16_t Socket::calcUDPChecksum(uint16_t dataLength, uint16_t dataChecksum)
{
	uint16_t headerChecksum = calcPseudoHeaderChecksum(IP_PROTO_UDP_V, dataLength + sizeof(UDPHeader));
	headerChecksum = Checksum::calc(headerChecksum, sizeof(UDPHeader), (uint8_t*)&packet.udp);

	return ~Checksum::add(headerChecksum, dataChecksum);
}


bool Socket::verifyUDPTCPChecksum()
{
#if ENABLE_UDPTCP_RX_CHECKSUM
	uint8_t headerLength;
	uint16_t dataOffset;
	switch (packet.ip.protocol)
	{
		case IP_PROTO_TCP_V:
		{
			headerLength = sizeof(IPHeader) + sizeof(TCPHeader);
			dataOffset = sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(TCPHeader);
		}break;
		case IP_PROTO_UDP_V:
		{
			headerLength = sizeof(IPHeader) + sizeof(UDPHeader);
			dataOffset = sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(UDPHeader);
		}break;
		default:
			return true;
	}

	uint16_t dataChecksum;

	uint16_t totalLength = packet.ip.totalLength.getValue();
	uint16_t dataLength = totalLength - headerLength;

	dataChecksum = Checksum::calc(dataLength, packet.raw + dataOffset);

	uint16_t sum;
	if (packet.ip.protocol == IP_PROTO_TCP_V)
		sum = calcTCPChecksum(false, dataLength, dataChecksum);
	else
		sum = calcUDPChecksum(dataLength, dataChecksum);

	return 0 == sum;


#else
	return true;
#endif

}

void Socket::setBroadcastRemoteIP()
{
	remoteIP = IPADDR_P(255, 255, 255, 255);
}