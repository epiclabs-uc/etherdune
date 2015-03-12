// EtherFlow ICMP Echo implementation
// Author: Javier Peletier <jm@friendev.com>
// Summary: Allows the application to send and respond to ICMP Echo (ping) requests
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
/// \brief Allows the application to send and respond to ICMP Echo (ping) requests
/// \details 
/// Implements the ICMP protocol. If you only want to have your application respond to ping requests,
/// instantiate 
/// 
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