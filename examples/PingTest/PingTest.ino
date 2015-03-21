// EtherDune ICMP Ping example
// Author: Javier Peletier <jm@friendev.com>
// Summary: Demonstrates how to use ICMPPinger and ICMPPingAutoReply classes
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

/// \file
/// \brief Demonstrates how to use ICMPPinger and ICMPPingAutoReply classes

/// \cond

#include <ACross.h>
#include <Checksum.h>
#include <TCPSocket.h>
#include <UDPSocket.h>

#include <inet.h>
#include <ENC28J60.h>
#include <DNS.h>
#include <FlowScanner.h>
#include <HTTPClient.h>
#include <ICMPPingAutoReply.h>
#include <ICMPPinger.h>
#include <DHCP.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("PingTest");

static const uint8_t CS_PIN = 10; //Put here what pin you are using for your ENC28J60's chip select
static MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };

DHCP dhcp;

// Instantiate ICMPPingAutoReply class, takes care of replying to incoming echo requests
ICMPPingAutoReply pingAutoReply; 

class ICMPHandler : ICMPPinger
{
	static const uint8_t PING_PERIOD = NTICKS(1000); //ms
	uint8_t timer;
	IPAddress target;

	void onPingReply(uint16_t time)
	{
		printf_P(PSTR("Reply from %d.%d.%d.%d: bytes=%d time=%dms TTL=%d\n"),
			packet.ip.sourceIP.b[0], packet.ip.sourceIP.b[1], packet.ip.sourceIP.b[2], packet.ip.sourceIP.b[3],
			packet.ip.totalLength - sizeof(IPHeader) - sizeof(ICMPHeader),
			time, packet.ip.TTL);

	}

	void tick()
	{
		timer--;

		if (timer == 0)
		{
			timer = PING_PERIOD;
			ping(target);
		}


		ICMPPinger::tick();
	}

public:
	void start(const IPAddress& targetIP)
	{
		timer = 1;
		target = targetIP;
		tick();
	}


}pingTest;


void setup()
{

	Serial.begin(115200);
	ACross::init();
#ifdef ACROSS_ARDUINO
	ACross::printf_serial_init();
#endif

	printf(PSTR("ICMP Ping EtherDune sample\n"));
	Serial.print(F("Free RAM: ")); Serial.println(ACross::getFreeRam());
	printf(PSTR("Press any key to start...\n"));

	while (!Serial.available());
	
	net::localMAC = mymac;

	if (!net::begin(CS_PIN))
		ACERROR("failed to start EtherDune");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");


	if (!dhcp.dhcpSetup())
	{
		Serial.println(F("DHCP setup failed"));
		ACross::halt(1);
	}

	Serial.println(F("DHCP setup OK"));

	printf_P(PSTR("Local IP is %d.%d.%d.%d. Try pinging me!\n\n"),
		net::localIP.b[0], net::localIP.b[1], net::localIP.b[2], net::localIP.b[3]);

	IPAddress targetIP;
	targetIP = IPADDR_P(8, 8, 8, 8);
	pingTest.start(targetIP);

}

void loop()
{
	net::loop();

}

/// \endcond