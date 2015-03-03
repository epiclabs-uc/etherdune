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
	
	if (chunk.eth.etherType.getValue() != ETHTYPE_ARP)
		return false;

	ACTRACE("processHeader");

	switch (chunk.arp.OPER.l)
	{
		case ARP_OPCODE_REPLY_L:
		{

			ACTRACE("ARP Reply=%02x:%02x:%02x:%02x:%02x:%02x", chunk.arp.senderMAC.b[0], chunk.arp.senderMAC.b[1], chunk.arp.senderMAC.b[2], chunk.arp.senderMAC.b[3], chunk.arp.senderMAC.b[4], chunk.arp.senderMAC.b[5]);
			processARPReply();

			return true;
		}break;

		case ARP_OPCODE_REQ_L:
		{
			ACTRACE("ARP Request from=%02x:%02x:%02x:%02x:%02x:%02x", chunk.arp.senderMAC.b[0], chunk.arp.senderMAC.b[1], chunk.arp.senderMAC.b[2], chunk.arp.senderMAC.b[3], chunk.arp.senderMAC.b[4], chunk.arp.senderMAC.b[5]);

			if (chunk.arp.targetIP.u == localIP.u)
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
	memset(&chunk.eth.dstMAC, 0xFF, sizeof(MACAddress));
	chunk.eth.srcMAC = chunk.arp.senderMAC = localMAC;
	chunk.eth.etherType.setValue(ETHTYPE_ARP);
	chunk.arp.HTYPE.setValue(0x0001);
	chunk.arp.PTYPE.setValue(0x0800);
	chunk.arp.HLEN = 0x06;
	chunk.arp.PLEN = 0x04;
	chunk.arp.OPER.setValue(0x0001);
	memset(&chunk.arp.targetMAC, 0x00, sizeof(MACAddress));
	chunk.arp.targetIP = ip;
	chunk.arp.senderIP = localIP;



	packetSend(sizeof(EthernetHeader) + sizeof(ARPPacket), chunk.raw);

}

void ARPService::makeARPReply()
{
	chunk.arp.targetMAC = chunk.eth.dstMAC = chunk.eth.srcMAC;
	chunk.arp.senderMAC = chunk.eth.srcMAC = localMAC;
	chunk.arp.OPER.l = ARP_OPCODE_REPLY_L;
	chunk.arp.targetIP = chunk.arp.senderIP;
	chunk.arp.senderIP = localIP;

	packetSend(sizeof(EthernetHeader) + sizeof(ARPPacket), chunk.raw);
}

void ARPService::processARPReply()
{
	int16_t lowest = MAX_ARP_TTL;
	ARPEntry * selectedEntry = NULL;
	for (ARPEntry* entry = arpTable + (ARP_TABLE_LENGTH - 1); entry >= arpTable; entry--)
	{
		if (entry->ip.u == chunk.arp.senderIP.u)
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
	selectedEntry->ip = chunk.arp.senderIP;
	selectedEntry->mac = chunk.arp.senderMAC;

}

