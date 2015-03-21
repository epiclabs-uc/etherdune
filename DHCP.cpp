// EtherDune DHCP Service
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements the basics of DHCP so as to obtain and maintain an IP lease
// along with DNS, gateway IP and netmask.
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

#include "DHCP.h"
#include "inet.h"
#include "DNS.h"

#define AC_LOGLEVEL 2
#include <ACLog.h>
ACROSS_MODULE("DHCP");

DHCP::DHCP()
{
	remotePort = 67;
	localPort = 68;
	state = DHCP_STATE_INIT;
}

void DHCP::prepareDHCPRequest()
{
	memset(&packet.dhcp, 0, sizeof(packet.dhcp));
	packet.dhcp.op = 1; //request
	packet.dhcp.htype = 1; //hardware type=ethernet;
	packet.dhcp.hlen = sizeof(MACAddress); //size of hardware address
	packet.dhcp.xid.rawValue = *(uint32_t*)&localMAC;  //transaction id, should be a random number. for now, use part of our MAC.
	packet.dhcp.broadcastFlag = 0x0080;
	packet.dhcp.mac = localMAC;
	setMagicCookie();

}

/// <summary>
/// Attempts to configure the IP settings: local IP, subnet mask, gateway and DNS via DHCP.
/// This call will block until EtherDune is configured or timeout and return false.
/// </summary>
/// <returns>true if successful, false otherwise.</returns>
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
	write(packet.dhcp);

	const DHCPDiscoverMessageTypeOption discover;

	write(discover);

	if (net::localIP.b[0]!= 0) //if the current IP address looks like it is valid, then try to request that one.
	{
		//DHCPRequestedIPOption requestIP;
		//requestIP.ip = net::localIP;
		//write(requestIP);

		//slower and uglier but saves 16bytes of flash.
		const DHCPOption<DHCP_OPTIONS_REQUESTED_IP, sizeof(IPAddress)> r;
		write(r);
		write(net::localIP);

		ACINFO("trying to request the same IP: %d.%d.%d.%d", net::localIP.b[0], net::localIP.b[1], net::localIP.b[2], net::localIP.b[3]);
	}

	writeAdditionalFields();


	write(DHCP_OPTIONS_END);

	remoteIP.setBroadcastIP();

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

	if (packet.dhcp.xid.rawValue != *(uint32_t*)&localMAC) //we're using part of our mac address as "transaction id"
		return;

	switch (state)
	{

		case DHCP_STATE_SELECTING:
		{
			if (getMessageType()!= DHCP_OFFER)
				return;

			DHCPIPOption* sioptionPtr = (DHCPIPOption*)findOption(DHCP_OPTIONS_SERVER_IDENTIFIER);
			if (sioptionPtr == NULL)
				return;

			DHCPIPOption serverIDOption = *sioptionPtr;
			DHCPRequestedIPOption requestedIPOption;
			requestedIPOption.ip = packet.dhcp.yiaddr;

			prepareDHCPRequest();
			write(packet.dhcp);
			
			const DHCPRequestMessageTypeOption request;
			write(request);
			write(serverIDOption);
			write(requestedIPOption);
			writeAdditionalFields();
			write(DHCP_OPTIONS_END);

			remoteIP.setBroadcastIP();

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
						if (timerOpt->timer.h.rawValue != 0)
							renewalTimer = 0xFFFF;
						else
							renewalTimer = timerOpt->timer.l;
					}
					else
						renewalTimer = DHCP_DEFAULT_RENEWAL_TIMER;

					net::netmask = subnetOpt->ip;
					net::dnsIP = dnsOpt->ip;
					net::gatewayIP = routerOpt->ip;
					
					net::localIP = packet.dhcp.yiaddr;
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
	packet.dhcp.magicCookie = IPADDR_P(99, 130, 83, 99);
}

DHCPOptionHeader* DHCP::findOption(uint8_t searchCode)
{

	DHCPOptionHeader* header = (DHCPOptionHeader*)&packet.dhcpOptions;
	for (;
		header->code != DHCP_OPTIONS_END &&
		(uint8_t*)header < (uint8_t*)&packet.dhcpOptions + sizeof(packet.dhcpOptions);
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

void DHCP::writeAdditionalFields()
{
	const DHCPClientIdentifierOptionHeader clientIdentifierHeader;
	write(clientIdentifierHeader);
	write(net::localMAC);

#if ENABLE_DHCP_HOSTNAME
	DHCPOptionHeader hostnameOptionHeader(DHCP_OPTIONS_HOSTNAME, strlen_P(DHCP_HOSTNAME));
	write(hostnameOptionHeader);
	write((const __FlashStringHelper*)DHCP_HOSTNAME);
#endif
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


