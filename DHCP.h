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
static const uint8_t DHCP_OPTIONS_REQUESTED_IP = 50;
static const uint8_t DHCP_OPTIONS_MESSAGETYPE = 53;
static const uint8_t DHCP_OPTIONS_SERVER_IDENTIFIER = 54;
static const uint8_t DHCP_OPTIONS_RENEWAL_TIME = 58;
static const uint8_t DHCP_OPTIONS_END = 255;


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


template <uint8_t MESSAGETYPE>
struct DHCPMessageTypeOption : public DHCPOption <DHCP_OPTIONS_MESSAGETYPE, 1>
{
	uint8_t messageType;
	DHCPMessageTypeOption() : messageType(MESSAGETYPE){}

};

typedef  DHCPMessageTypeOption<DHCP_DISCOVER> DHCPDiscoverMessageTypeOption;
typedef  DHCPMessageTypeOption<DHCP_REQUEST> DHCPRequestMessageTypeOption;





class DHCP : public UDPSocket, Stateful
{
private:

	uint8_t attempts;
	uint16_t renewalTimer;

	void onReceive(uint16_t len);
	void setMagicCookie();
	void sendDHCPDiscover();
	void initDHCP();

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