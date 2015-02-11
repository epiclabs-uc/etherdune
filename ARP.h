#ifndef __ARP_H_
#define __ARP_H_

#include "NetworkService.h"

class ARPService : NetworkService
{
	static ARPEntry arpTable[ARP_TABLE_LENGTH];

	bool processHeader();
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