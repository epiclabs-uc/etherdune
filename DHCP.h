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

/// \class DHCP
/// \brief EtherDune DHCP Service
/// \details Implements the basics of DHCP so as to obtain and maintain an IP lease
/// along with DNS, gateway IP and netmask.
/// See \ref DHCPConfig for EtherDune config parameters that apply to DHCP.
///
/// EtherDune DHCP is a crude "straight to the point" implementation of DHCP. 
/// It does not implement a full RENEW/REBIND cycle, but rather shortcuts the whole process
/// fully renewing the lease once the timeout expires. This simplifies the implementation
/// saving valuable flash/code space.
///
/// In order to use DHCP in your project, instantiate an object of this class, call
/// \ref dhcpSetup and keep the object instance alive for as long
/// as you want to keep the lease active.
///
/// If you do not instantiate the \ref DHCP class in your code, it will not be compiled in, therefore
/// not affecting your code size if unused.

#ifndef _DHCP_H_
#define _DHCP_H_

#include <ACross.h>
#include "UDPSocket.h"
#include "Stateful.h"


static const uint8_t DHCP_DISCOVER = 1;
static const uint8_t DHCP_OFFER = 2;
static const uint8_t DHCP_REQUEST = 3;
static const uint8_t DHCP_DECLINE = 4;
static const uint8_t DHCP_ACK = 5;
static const uint8_t DHCP_NACK = 6;
static const uint8_t DHCP_RELEASE = 7;
static const uint8_t DHCP_INFORM = 8;

static const uint8_t DHCP_OPTIONS_PAD = 0;
static const uint8_t DHCP_OPTIONS_SUBNET = 1;
static const uint8_t DHCP_OPTIONS_ROUTER = 3;
static const uint8_t DHCP_OPTIONS_DNS = 6;
static const uint8_t DHCP_OPTIONS_HOSTNAME = 12;
static const uint8_t DHCP_OPTIONS_REQUESTED_IP = 50;
static const uint8_t DHCP_OPTIONS_MESSAGETYPE = 53;
static const uint8_t DHCP_OPTIONS_SERVER_IDENTIFIER = 54;
static const uint8_t DHCP_OPTIONS_RENEWAL_TIME = 58;
static const uint8_t DHCP_OPTIONS_CLIENT_IDENTIFIER = 61;
static const uint8_t DHCP_OPTIONS_END = 255;

static const uint8_t DHCP_HARDWARE_TYPE_ETHERNET = 1;

/// \cond
struct DHCPOptionHeader
{
	uint8_t code;
	uint8_t length;
	DHCPOptionHeader(uint8_t optionCode, uint8_t optionLength) : code(optionCode), length(optionLength){}
	DHCPOptionHeader(){}

};

template<uint8_t CODE, uint8_t LENGTH>
struct DHCPOption : public DHCPOptionHeader
{
	DHCPOption() : DHCPOptionHeader(CODE, LENGTH){}

};

struct DHCPIPOption : DHCPOptionHeader
{
	IPAddress ip;
};

struct DHCPRequestedIPOption : public DHCPOption < DHCP_OPTIONS_REQUESTED_IP, sizeof(IPAddress) >
{
	IPAddress ip;
};

struct DHCPTimerOption :DHCPOptionHeader
{
	nint32_t timer;
};

struct DHCPClientIdentifierOptionHeader : DHCPOption<DHCP_OPTIONS_CLIENT_IDENTIFIER, sizeof(uint8_t) + sizeof(MACAddress)>
{
	uint8_t hardwareType;
	DHCPClientIdentifierOptionHeader() :hardwareType(DHCP_HARDWARE_TYPE_ETHERNET){}
};




template <uint8_t MESSAGETYPE>
struct DHCPMessageTypeOption : public DHCPOption <DHCP_OPTIONS_MESSAGETYPE, 1>
{
	uint8_t messageType;
	DHCPMessageTypeOption() : messageType(MESSAGETYPE){}

};

typedef  DHCPMessageTypeOption<DHCP_DISCOVER> DHCPDiscoverMessageTypeOption;
typedef  DHCPMessageTypeOption<DHCP_REQUEST> DHCPRequestMessageTypeOption;



/// \endcond

class DHCP : private UDPSocket, Stateful
{
private:

	uint8_t attempts;
	uint16_t renewalTimer;

	void onReceive(uint16_t len);
	void setMagicCookie();
	void sendDHCPDiscover();
	void initDHCP();
	void writeAdditionalFields();
	NOINLINE DHCPOptionHeader* findOption(uint8_t searchCode);
	void prepareDHCPRequest();
	uint8_t getMessageType();
	__FlashStringHelper* getStateString();
	void tick();

public:

	DHCP();
	bool dhcpSetup();

};







#endif