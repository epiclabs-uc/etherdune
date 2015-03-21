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

/// \class Socket
/// \brief Base class for TCP and UDP sockets
/// \details
/**

This class contains all the code common to TCP and UDP implementations, 
such as checksum calculation/verification and write functions

*/

#ifndef _SOCKET_h
#define _SOCKET_h

#include <ACross.h>

#include "SharedBuffer.h"
#include "NetworkService.h"

class Socket;


class Socket : protected NetworkService
{

 protected:

	SharedBuffer buffer; //!< output buffer for this socket
	static uint8_t srcPort_L_count; //!< self-incrementing counter for local ports.
	
	void prepareIPPacket();
	static uint16_t calcPseudoHeaderChecksum(uint8_t protocol, uint16_t length);
	static uint16_t calcTCPChecksum(bool options, uint16_t dataLength, uint16_t dataChecksum);
	static uint16_t calcUDPChecksum(uint16_t dataLength, uint16_t dataChecksum);
	static bool verifyUDPTCPChecksum();

 public:

	 nint16_t remotePort; //!< remote TCP or UDP port
	 nint16_t localPort; //!< local TCP or UDP port
	 IPAddress remoteIP; //!< remote IP address to connect to (TCP) or send the next packet to (UDP)

	 uint16_t write(uint16_t len, const byte* data);
	 uint16_t write(const String& s);
	 uint16_t write(const __FlashStringHelper* pattern, ...);

	 /// <summary>
	 /// Writes out the binary representation of the parameter to the socket.
	 /// If used with a null-terminated string constant, it will output the string **including**
	 /// the null-terminating character \0.
	 ///
	 /// If used with datatypes such as int, it will output the actual binary representation of
	 /// an integer, not the ASCII visual representation of it.
	 ///
	 /// Useful to serialize structs or complex types for sending.
	 ///
	 /// See UDPClientDemo_NTPClient.ino for an example usage
	 /// </summary>
	 /// <param name="message">The message.</param>
	 /// <returns>Number of bytes written</returns>
	 /// <example><code>
	 /// struct 
	 /// {
	 ///   int status;
	 ///   byte code;
	 /// } message;
	 ///
	 /// message.status = 200;
	 /// message.code = 10;
	 /// socket.write(message);
	 /// </code></example>
	 template <class T>
	 inline uint16_t write(const T& message)
	 {
		 return write(sizeof(T), (byte*)&message);
	 }

};




#endif

