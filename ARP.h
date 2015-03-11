// EtherFlow ARP Service
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements the ARP protocol as an EtherFlow service
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

#ifndef __ARP_H_
#define __ARP_H_

#include "NetworkService.h"

class ARPService : NetworkService
{
	static ARPEntry arpTable[ARP_TABLE_LENGTH];

	bool onPacketReceived();
	bool processChunk(bool isHeader);
	void tick();

	static void makeWhoHasARPRequest(IPAddress& ip);
	static void makeARPReply();
	static void processARPReply();

public:
	
	MACAddress* whoHas(IPAddress& ip);
	ARPService();

};




#endif