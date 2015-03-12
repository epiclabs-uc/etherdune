// EtherFlow ENC28J60 hardware driver
// Author: Javier Peletier <jm@friendev.com>
// Credits: Initially based off EtherCard's own enc28j60.cpp file, but heavily modified afterwards.
// Credits: Jean-Claude Wippler, Guido Socher and Pascal Stang
// Summary: Encapsulates access to the ENC28J60 hardware
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

/// \class ENC28J60
/// \brief Encapsulates access to the ENC28J60 hardware
/// \details Low-level class to access hardware functions

#ifndef __EENC28J60__H_
#define __EENC28J60__H_

#define __PROG_TYPES_COMPAT__

#include <ACross.h>

#include "inet.h"
#include "enc28j60constants.h"
#include "config.h"


class NetworkService;
class SharedBuffer;

class ENC28J60 
{
	friend class SharedBuffer;

protected:

	static void writeByte(byte b);
	static void writeByte(uint16_t dst, byte b);
	static void writeBuf(uint16_t dst, uint16_t len, const byte* data);
	static void writeBuf(uint16_t len, const byte* data);
	static uint16_t hardwareChecksum(uint16_t src, uint16_t len);
	static uint16_t hardwareChecksumRxOffset(uint16_t offset, uint16_t len);
	static void moveMem(uint16_t dest, uint16_t src, uint16_t len);
	static void readBuf(uint16_t src, uint16_t len, byte* data);
	static void readBuf(uint16_t len, byte* data);
	static byte readByte(uint16_t src);
	static void packetSend(uint16_t len);
	static void packetSend(uint16_t len, const byte* data);
	static void loadSample();
	static void loadAll();
	static void release();
	static uint8_t begin(uint8_t cspin);
	static void loop();
	static void enableBroadcast();

public:

	static bool isLinkUp();
	
};

#endif
