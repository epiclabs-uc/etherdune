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
static const uint8_t DHCP_OPTIONS_END = 255;


struct DHCPOptionHeader
{
	uint8_t code;
	uint8_t lenght;
	DHCPOptionHeader(uint8_t optionCode, uint8_t optionLength) : code(optionCode), lenght(optionLength){}

};

template<uint8_t CODE, uint8_t LENGTH>
struct DHCPOption : public DHCPOptionHeader
{
	DHCPOption() : DHCPOptionHeader(CODE, LENGTH){}

};

template <uint8_t MESSAGETYPE>
struct DHCPMessageTypeOption : public DHCPOption <53, 1>
{
	uint8_t messageType;
	DHCPMessageTypeOption() : messageType(MESSAGETYPE){}

};

typedef  DHCPMessageTypeOption<DHCP_DISCOVER> DHCPDiscoverMessageTypeOption;





class DHCP : public UDPSocket, Stateful
{
private:
	bool onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data);

public:

	DHCP();
	void dhcpSetup();




};







#endif