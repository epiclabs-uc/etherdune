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
static const uint8_t DHCP_OPTIONS_REQUESTED_IP = 50;
static const uint8_t DHCP_OPTIONS_MESSAGETYPE = 53;
static const uint8_t DHCP_OPTIONS_SERVER_IDENTIFIER = 54;
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

struct DHCPServerIPOption : DHCPOptionHeader
{
	IPAddress ip;
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
	void onReceive(uint16_t len);
	void setMagicCookie();
	DHCPOptionHeader* findOption(uint8_t searchCode);
	void prepareDHCPRequest();
	uint8_t getMessageType();

public:

	DHCP();
	void dhcpSetup();




};







#endif