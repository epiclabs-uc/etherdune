#include "DHCP.h"
#include "inet.h"


DHCP::DHCP()
{
	remotePort.setValue(67);
	localPort.setValue(68);
	state = DHCP_STATE_INIT;
}

void DHCP::dhcpSetup()
{
	chunk.dhcp0.op = 1; //request
	chunk.dhcp0.htype = 1; //hardware type=ethernet;
	chunk.dhcp0.hlen = sizeof(MACAddress); //size of hardware address
	chunk.dhcp0.hops = 0;
	chunk.dhcp0.xid.l.l = 1; //transaction id, should be a random number.
	chunk.dhcp0.secs.zero();
	chunk.dhcp0.broadcastFlag = 0x0080;
	chunk.dhcp0.ciaddr.u = 0;
	chunk.dhcp0.yiaddr.u = 0;
	chunk.dhcp0.siaddr.u = 0;
	chunk.dhcp0.giaddr.u = 0;
	chunk.dhcp0.mac = localMAC;

	remoteIP = IPADDR_P(255, 255, 255, 255);

	write(chunk.dhcp0);
	chunk.dhcp1.sname[0] = 0;
	write(chunk.dhcp1);
	write(chunk.dhcp2);
	write(chunk.dhcp3);
	IPAddress magicCookie;
	magicCookie = IPADDR_P(99, 130, 83, 99);
	write(magicCookie);

	const DHCPDiscoverMessageTypeOption discover;

	write(discover);

	write(DHCP_OPTIONS_END);

	send();

}

bool DHCP::onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data)
{



}