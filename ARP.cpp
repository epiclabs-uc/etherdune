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

#include "ARP.h"

#define AC_LOGLEVEL 2
#include <ACLog.h>
ACROSS_MODULE("ARP");

static uint16_t minuteTimer = 60 * 1000 / NETWORK_TIMER_RESOLUTION;

ARPEntry ARPService::arpTable[ARP_TABLE_LENGTH];



ARPService::ARPService()
{
	memset(arpTable, -2, ARP_TABLE_LENGTH * sizeof(ARPEntry));
}

bool ARPService::onPacketReceived()
{
	
	if (packet.eth.etherType != ETHTYPE_ARP)
		return false;

	ACTRACE("processHeader");

	switch (packet.arp.OPER.l)
	{
		case ARP_OPCODE_REPLY_L:
		{

			ACTRACE("ARP Reply=%02x:%02x:%02x:%02x:%02x:%02x", packet.arp.senderMAC.b[0], packet.arp.senderMAC.b[1], packet.arp.senderMAC.b[2], packet.arp.senderMAC.b[3], packet.arp.senderMAC.b[4], packet.arp.senderMAC.b[5]);
			processARPReply();

			return true;
		}break;

		case ARP_OPCODE_REQ_L:
		{
			ACTRACE("ARP Request from=%02x:%02x:%02x:%02x:%02x:%02x", packet.arp.senderMAC.b[0], packet.arp.senderMAC.b[1], packet.arp.senderMAC.b[2], packet.arp.senderMAC.b[3], packet.arp.senderMAC.b[4], packet.arp.senderMAC.b[5]);

			if (packet.arp.targetIP.u == localIP.u)
				makeARPReply();
			return true;
		}break;

		default:
		{
			return true;
		}break;
	}
}


void ARPService::tick()
{
	minuteTimer--;

	if (minuteTimer == 0)
	{
		minuteTimer = 60 * 1000 / NETWORK_TIMER_RESOLUTION;

		for (ARPEntry* entry = arpTable + (ARP_TABLE_LENGTH - 1); entry >= arpTable; entry--)
		{
			if (entry->status_TTL > 0)
				entry->status_TTL--;
		}
	}
}


/// <summary>
/// Attempts to return the MAC address associated with the provided IP address.
/// If the result is found in the cache, it is returned, otherwise, an ARP
/// query is issued and whoHas returns NULL
/// </summary>
/// <param name="ip">IP to search</param>
/// <returns>NULL if the IP address was not in the cache (a query is immediately issued). Otherwise, a pointer
/// to a MAC address within the ARP table.
/// </returns>
MACAddress* ARPService::whoHas(IPAddress& ip)
{
	for (ARPEntry* entry = arpTable + (ARP_TABLE_LENGTH - 1); entry >= arpTable; entry--)
	{
		if (ip.u == entry->ip.u && entry->status_TTL > 0)
		{
			entry->status_TTL = MAX_ARP_TTL;
			return &entry->mac;
		}
	}

	makeWhoHasARPRequest(ip);

	return NULL;


}



void ARPService::makeWhoHasARPRequest(IPAddress& ip)
{
	memset(&packet.eth.dstMAC, 0xFF, sizeof(MACAddress));
	packet.eth.srcMAC = packet.arp.senderMAC = localMAC;
	packet.eth.etherType=ETHTYPE_ARP;
	packet.arp.HTYPE = 0x0001;
	packet.arp.PTYPE = 0x0800;
	packet.arp.HLEN = 0x06;
	packet.arp.PLEN = 0x04;
	packet.arp.OPER = 0x0001;
	memset(&packet.arp.targetMAC, 0x00, sizeof(MACAddress));
	packet.arp.targetIP = ip;
	packet.arp.senderIP = localIP;



	packetSend(sizeof(EthernetHeader) + sizeof(ARPPacket), packet.raw);

}

void ARPService::makeARPReply()
{
	packet.arp.targetMAC = packet.eth.dstMAC = packet.eth.srcMAC;
	packet.arp.senderMAC = packet.eth.srcMAC = localMAC;
	packet.arp.OPER.l = ARP_OPCODE_REPLY_L;
	packet.arp.targetIP = packet.arp.senderIP;
	packet.arp.senderIP = localIP;

	packetSend(sizeof(EthernetHeader) + sizeof(ARPPacket), packet.raw);
}

void ARPService::processARPReply()
{
	int16_t lowest = MAX_ARP_TTL;
	ARPEntry * selectedEntry = NULL;
	for (ARPEntry* entry = arpTable + (ARP_TABLE_LENGTH - 1); entry >= arpTable; entry--)
	{
		if (entry->ip.u == packet.arp.senderIP.u)
		{
			selectedEntry = entry;
			break;
		}

		if (entry->status_TTL <= lowest)
		{
			lowest = entry->status_TTL;
			selectedEntry = entry;
		}
	}

	ACBREAK(selectedEntry!=NULL,"selectedEntry is NULL");

	selectedEntry->status_TTL = MAX_ARP_TTL;
	selectedEntry->ip = packet.arp.senderIP;
	selectedEntry->mac = packet.arp.senderMAC;

}

