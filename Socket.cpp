// EtherDune Socket base class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Base class for TCP and UDP sockets
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

#include "Socket.h"
#include "Checksum.h"

#define AC_LOGLEVEL 2
#include <ACLog.h>
ACROSS_MODULE("Socket");

uint8_t Socket::srcPort_L_count = 0;



void Socket::prepareIPPacket()
{
	NetworkService::prepareIPPacket(remoteIP);
	//packet.ip.version = 4;
	//packet.ip.IHL = 0x05; //20 bytes
	//packet.ip.raw[1] = 0x00; //DSCP/ECN=0;
	//packet.ip.identification.zero();
	//packet.ip.flags = 0;
	//packet.ip.fragmentOffset = 0;
	//packet.ip.checksum.zero();
	//packet.ip.sourceIP = localIP;
	//packet.ip.destinationIP = remoteIP;
	//packet.ip.TTL = 255;
	//packet.ip.checksum.rawValue = ~Checksum::calc(sizeof(IPHeader), (uint8_t*)&packet.ip);
}




/// <summary>
/// In the case of TCP, writes the given data buffer to the socket.
///
/// For UDP sockets, appends the given data buffer to the current packet being built
/// </summary>
/// <param name="len">length of the data buffer</param>
/// <param name="data">pointer to the data to send</param>
/// <returns>Number of bytes written</returns>
uint16_t Socket::write(uint16_t len, const byte* data)
{
	ACTRACE("write %d bytes. Dump:",len )
	AC_TRACE(Serial.write(data, len));
	AC_TRACE(Serial.println());

	return buffer.write(len, data);
}

/// <summary>
/// In the case of TCP, writes the given String to the socket, not including the null-terminating character.
///
/// For UDP sockets, appends the given String to the current packet being built, not including the null-terminating character.
/// </summary>
/// <param name="s">String to write out</param>
/// <returns>Number of bytes written</returns>
uint16_t Socket::write(const String& s)
{
	return write(s.length(), (uint8_t*)s.c_str());
}

/// <summary>
/// Writes out the specified PROGMEM string that may contain the % escape character. For each
/// % found, the next argument in the variable arguments list is output instead.
/// Variable arguments must be of type String*.
/// To actually write a %, put two %%.
/// </summary>
/// <param name="pattern">Pattern PROGMEM string. You can produce this string 
/// by using the F() macro</param>
/// <param name="...">Optional substitution String* pointers, one for each % in the pattern string</param>
/// <returns>Number of bytes written</returns>
/// <example>
/// <code>
/// String status(200);
/// String statusMessage(F("OK"));
/// String contentType(F("text/html"));
/// String message(F("Hello World!"));
/// 
/// socket.write(F("HTTP % %\r\nContent-Type:%\r\n\r\n&lt;html&gt;&lt;body&gt;&lt;h1&gt;%&lt;/h1&gt;&lt;/body&gt;&lt;/html&gt;"), 
///	    &status, &statusMessage, &contentType, &message);
/// </code>
/// Output:
/// <code>
/// HTTP 200 OK
/// Content-Type:text/html
///
/// &lt;html&gt;&lt;body&gt;&lt;h1&gt;Hello World!&lt;/h1&gt;&lt;/body&gt;&lt;/html&gt;
/// </code>
/// </example>
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
			bytes += write(i, (uint8_t*)buf);
			va_end(args);
			return bytes;
		}

		buf[i] = c;
		i++;
		if (i == sizeof(buf))
		{
			bytes += write(i, (uint8_t*)buf);
			i = 0;

		}


	}

}

/// <summary>
/// Calculates the TCP or UDP pseudo header checksum.
/// </summary>
/// <param name="protocol">protocol code, either IP_PROTO_TCP or IP_PROTO_UDP</param>
/// <param name="length">Total length of the frame: UDP/TCP header and data</param>
/// <returns>The calculated checksum</returns>
/// <remarks>See more about the TCP/UDP pseudo header checksum here: http://en.wikipedia.org/wiki/Transmission_Control_Protocol#Checksum_computation
uint16_t Socket::calcPseudoHeaderChecksum(uint8_t protocol, uint16_t length)
{
	nint32_t pseudo;
	pseudo.h.h = 0;
	pseudo.h.l = protocol;
	pseudo.l=length;

	uint16_t sum = Checksum::calc(sizeof(IPAddress) * 2, (uint8_t*)&packet.ip.sourceIP);
	return Checksum::calc(sum, sizeof(pseudo), (uint8_t*)&pseudo);
}

/// <summary>
/// Calculates the TCP checksum.
/// </summary>
/// <param name="options">if set to <c>true</c>, considers extra size of a 2 byte length
/// option, used in EtherDune to establish the MSS</param>
/// <param name="dataLength">Length of the payload</param>
/// <param name="dataChecksum">Checksum of the payload</param>
/// <returns>The calculated checksum of the TCP segment</returns>
uint16_t Socket::calcTCPChecksum(bool options, uint16_t dataLength, uint16_t dataChecksum)
{
	uint8_t headerLength = options ? sizeof(TCPOptions) + sizeof(TCPHeader) : sizeof(TCPHeader);
	uint16_t sum = calcPseudoHeaderChecksum(IP_PROTO_TCP, dataLength + headerLength);
	sum = Checksum::calc(sum, headerLength, (uint8_t*)&packet.tcp);
	sum = Checksum::add(sum, dataChecksum);
	return ~sum;
}

/// <summary>
/// Calculates the UDP checksum.
/// </summary>
/// <param name="dataLength">Length of the payload.</param>
/// <param name="dataChecksum">Checksum of the payload</param>
/// <returns>The calculated checksum of the UDP datagram</returns>
uint16_t Socket::calcUDPChecksum(uint16_t dataLength, uint16_t dataChecksum)
{
	uint16_t headerChecksum = calcPseudoHeaderChecksum(IP_PROTO_UDP, dataLength + sizeof(UDPHeader));
	headerChecksum = Checksum::calc(headerChecksum, sizeof(UDPHeader), (uint8_t*)&packet.udp);

	return ~Checksum::add(headerChecksum, dataChecksum);
}


/// <summary>
/// Verifies if the UDP or TCP checksum of the current packet is correct.
/// </summary>
/// <returns><c>true</c> if the checksum is correct, <c>false</c> otherwise</returns>
bool Socket::verifyUDPTCPChecksum()
{
#if ENABLE_UDPTCP_RX_CHECKSUM
	uint8_t headerLength;
	uint16_t dataOffset;
	switch (packet.ip.protocol)
	{
		case IP_PROTO_TCP:
		{
			headerLength = sizeof(IPHeader) + sizeof(TCPHeader);
			dataOffset = sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(TCPHeader);
		}break;
		case IP_PROTO_UDP:
		{
			headerLength = sizeof(IPHeader) + sizeof(UDPHeader);
			dataOffset = sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(UDPHeader);
		}break;
		default:
			return true;
	}

	uint16_t dataChecksum;

	uint16_t totalLength = packet.ip.totalLength;
	uint16_t dataLength = totalLength - headerLength;

	dataChecksum = Checksum::calc(dataLength, packet.raw + dataOffset);

	uint16_t sum;
	if (packet.ip.protocol == IP_PROTO_TCP)
		sum = calcTCPChecksum(false, dataLength, dataChecksum);
	else
		sum = calcUDPChecksum(dataLength, dataChecksum);

	return 0 == sum;


#else
	return true;
#endif

}

