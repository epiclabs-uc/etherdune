#include "DHCP.h"
#include "inet.h"
#include "DNS.h"

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

bool DHCP::dhcpSetup()
{
	attempts = DHCP_MAX_ATTEMPTS;
	initDHCP();

	while (attempts > 0 && state != DHCP_STATE_BOUND)
		net::loop();

	return state == DHCP_STATE_BOUND;
}

void DHCP::sendDHCPDiscover()
{
	prepareDHCPRequest();
	write(chunk.dhcp);

	const DHCPDiscoverMessageTypeOption discover;

	write(discover);

	if (net::localIP.b[0]!= 0) //if the current IP address looks like it is valid, then try to request that one.
	{
		DHCPRequestedIPOption requestIP;
		requestIP.ip = net::localIP;
		write(requestIP);
		ACINFO("trying to request the same IP: %d.%d.%d.%d", net::localIP.b[0], net::localIP.b[1], net::localIP.b[2], net::localIP.b[3]);
	}

	write(DHCP_OPTIONS_END);

	setBroadcastRemoteIP();

	send();
	
}

void DHCP::initDHCP()
{
	attempts--;
	sendDHCPDiscover();
	setState(DHCP_STATE_SELECTING, DHCP_TIMEOUT_SELECTING);
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
		case DHCP_STATE_RENEWING:
		case DHCP_STATE_SELECTING:
		{
			if (getMessageType()!= DHCP_OFFER)
				return;

			DHCPIPOption* sioptionPtr = (DHCPIPOption*)findOption(DHCP_OPTIONS_SERVER_IDENTIFIER);
			if (sioptionPtr == NULL)
				return;

			DHCPIPOption serverIDOption = *sioptionPtr;
			DHCPRequestedIPOption requestedIPOption;
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
					DHCPIPOption* subnetOpt = (DHCPIPOption*)findOption(DHCP_OPTIONS_SUBNET);
					DHCPIPOption* dnsOpt = (DHCPIPOption*)findOption(DHCP_OPTIONS_DNS);
					DHCPIPOption* routerOpt = (DHCPIPOption*)findOption(DHCP_OPTIONS_ROUTER);
					if (subnetOpt == NULL || dnsOpt==NULL || routerOpt== NULL)
						return;

					DHCPTimerOption* timerOpt = (DHCPTimerOption*)findOption(DHCP_OPTIONS_RENEWAL_TIME);
					if (timerOpt != NULL)
					{
						if (timerOpt->timer.h.rawu != 0)
							renewalTimer = 0xFFFF;
						else
							renewalTimer = timerOpt->timer.l.getValue();
					}
					else
						renewalTimer = DHCP_DEFAULT_RENEWAL_TIMER;

					renewalTimer = 10;

					net::netmask = subnetOpt->ip;
					net::DNS.serverIP() = dnsOpt->ip;
					net::gatewayIP = routerOpt->ip;
					
					net::localIP = chunk.dhcp.yiaddr;
					setState(DHCP_STATE_BOUND, DHCP_TIMEOUT_BOUND);

					ACINFO("IP:%d.%d.%d.%d", net::localIP.b[0], net::localIP.b[1], net::localIP.b[2], net::localIP.b[3]);
					ACINFO("Subnet:%d.%d.%d.%d", net::netmask.b[0], net::netmask.b[1], net::netmask.b[2], net::netmask.b[3]);
					ACINFO("DNS:%d.%d.%d.%d", net::DNS.serverIP().b[0], net::DNS.serverIP().b[1], net::DNS.serverIP().b[2], net::DNS.serverIP().b[3]);
					ACINFO("Gateway:%d.%d.%d.%d", net::gatewayIP.b[0], net::gatewayIP.b[1], net::gatewayIP.b[2], net::gatewayIP.b[3]);

				}break;

				case DHCP_NACK:
				{
					initDHCP();//start over

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

void DHCP::tick()
{

	ACDEBUG("tick/state=%S", getStateString());

	if (stateTimer == 1) //handle timeouts
	{
		switch (state)
		{

			case DHCP_STATE_BOUND:
			{
				stateTimer = DHCP_TIMEOUT_BOUND;
				renewalTimer--;
				if (renewalTimer > 0)
					goto tick_end;
				
				//renewal timer ran out, fall back bellow to initDHCP();
				
			}
			case DHCP_STATE_SELECTING:
			case DHCP_STATE_REQUESTING:
			{
				initDHCP();
				goto tick_end;

			}break;
		}
	}

	if (stateTimer>0)
		stateTimer--;

	switch (state)
	{
		case DHCP_STATE_SELECTING:
		{
			sendDHCPDiscover(); //resend message every tick.
		}break;

	}

	tick_end:
	UDPSocket::tick();
}




__FlashStringHelper* DHCP::getStateString()
{
	const char* s;

	switch (state)
	{
		case DHCP_STATE_BOUND: s = PSTR("BOUND"); break;
		case DHCP_STATE_INIT: s = PSTR("INIT"); break;
		case DHCP_STATE_REBINDING: s = PSTR("REBINDING"); break;
		case DHCP_STATE_RENEWING: s = PSTR("RENEWING"); break;
		case DHCP_STATE_REQUESTING: s = PSTR("REQUESTING"); break;
		case DHCP_STATE_SELECTING: s = PSTR("SELECTING"); break;

		default:
			s = PSTR("UNKNOWN");
	}

	return (__FlashStringHelper*)s;

}