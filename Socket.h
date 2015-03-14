// EtherFlow Socket base class
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

#ifndef _SOCKET_h
#define _SOCKET_h

#include <ACross.h>

#include "SharedBuffer.h"
#include "NetworkService.h"

class Socket;


class Socket : protected NetworkService
{

 protected:

	SharedBuffer buffer;
	static uint8_t srcPort_L_count;
	
	void prepareIPPacket();
	static uint16_t calcPseudoHeaderChecksum(uint8_t protocol, uint16_t length);
	static uint16_t calcTCPChecksum(bool options, uint16_t dataLength, uint16_t dataChecksum);
	static uint16_t calcUDPChecksum(uint16_t dataLength, uint16_t dataChecksum);
	static bool verifyTCPChecksum();
	static bool verifyUDPChecksum();
	static uint16_t calcDataChecksum(uint16_t& length, uint16_t dataOffset);
	void setBroadcastRemoteIP();

 public:

	 nint16_t remotePort;
	 nint16_t localPort;
	 IPAddress remoteIP;

	 uint16_t write(uint16_t len, const byte* data);
	 uint16_t write(const String& s);
	 uint16_t write(const __FlashStringHelper* pattern, ...);

	 template <class T>
	 inline uint16_t write(const T& message)
	 {
		 return write(sizeof(T), (byte*)&message);
	 }

};




#endif

