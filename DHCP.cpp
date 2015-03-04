#include "DHCP.h"
#include "inet.h"

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("DHCP");

DHCP::DHCP()
{
	remotePort.setValue(67);
	localPort.setValue(68);
	state = DHCP_STATE_INIT;
}

void DHCP::prepareDHCPRequest()
{
	memset(&chunk.dhcp, 0, sizeof(chunk.dhcp));
	chunk.dhcp.op = 1; //request
	chunk.dhcp.htype = 1; //hardware type=ethernet;
	chunk.dhcp.hlen = sizeof(MACAddress); //size of hardware address
	chunk.dhcp.xid.rawu = *(uint32_t*)&localMAC;  //transaction id, should be a random number. for now, use part of our MAC.
	chunk.dhcp.broadcastFlag = 0x0080;
	chunk.dhcp.mac = localMAC;
	setMagicCookie();

}

void DHCP::dhcpSetup()
{
	prepareDHCPRequest();
	write(chunk.dhcp);

	const DHCPDiscoverMessageTypeOption discover;

	write(discover);

	write(DHCP_OPTIONS_END);

	setBroadcastRemoteIP();

	send();
	setState(DHCP_STATE_SELECTING,DHCP_TIMEOUT_SELECTING);
}

uint8_t DHCP::getMessageType()
{
	DHCPMessageTypeOption<0>* mtoption = (DHCPMessageTypeOption<0>*)findOption(DHCP_OPTIONS_MESSAGETYPE);

	if (mtoption == NULL)
		return 0;
	else
		return mtoption->messageType;
}

void DHCP::onReceive(uint16_t len)
{
	ACTRACE("DHCP response");

	if (chunk.dhcp.xid.rawu != *(uint32_t*)&localMAC) //we're using part of our mac address as "transaction id"
		return;

	switch (state)
	{
		case DHCP_STATE_SELECTING:
		{
			if (getMessageType()!= DHCP_OFFER)
				return;

			DHCPServerIPOption* sioptionPtr = (DHCPServerIPOption*)findOption(DHCP_OPTIONS_SERVER_IDENTIFIER);
			if (sioptionPtr == NULL)
				return;

			DHCPServerIPOption serverIDOption = *sioptionPtr;
			DHCPServerIPOption requestedIPOption;
			requestedIPOption.ip = chunk.dhcp.yiaddr;

			prepareDHCPRequest();
			write(chunk.dhcp);
			
			const DHCPRequestMessageTypeOption request;
			write(request);
			write(serverIDOption);
			write(requestedIPOption);
			write(DHCP_OPTIONS_END);

			setBroadcastRemoteIP();

			send();
			setState(DHCP_STATE_REQUESTING, DHCP_TIMEOUT_REQUESTING);

		}break;

		case DHCP_STATE_REQUESTING:
		{
			switch (getMessageType())
			{
				case DHCP_ACK:
				{

				}break;

			}
				

		}break;
	}


}

void DHCP::setMagicCookie()
{
	chunk.dhcp.magicCookie = IPADDR_P(99, 130, 83, 99);
}

DHCPOptionHeader* DHCP::findOption(uint8_t searchCode)
{

	DHCPOptionHeader* header = (DHCPOptionHeader*)&chunk.dhcpOptions;
	for (;
		header->code != DHCP_OPTIONS_END &&
		(uint8_t*)header < (uint8_t*)&chunk.dhcpOptions + sizeof(chunk.dhcpOptions);
		header = (DHCPOptionHeader*)(((uint8_t*)header) + header->length+sizeof(DHCPOptionHeader)))
	{
		if (header->code == searchCode)
			return header;

	}

	return ((searchCode==header->code) ? header : NULL);

}