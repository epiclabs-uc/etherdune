// EtherFlow ICMP Abstract base class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements common ICMP packet building routines and ICMP checksum
// Summary: Used as base class for ICMP Echo (ping)
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

/// \class ICMP
/// \brief Implements common ICMP packet building routines and ICMP checksum
/// \details 
/// Implements code common to all ICMP services, such as checksum and packet preparation
///
/// This is an abstract base class, a building block for any ICMP-based service.
///

#ifndef _ICMP_H_
#define _ICMP_H_

#include <ACross.h>
#include "NetworkService.h"

class ICMP : protected NetworkService
{

private:
	
	bool onPacketReceived();

protected:

	bool loadAll();
	void calcICMPChecksum();
	void sendICMPPacket(const IPAddress& targetIP, uint16_t dataLength);

	virtual bool onICMPMessage() = 0;

};

#endif