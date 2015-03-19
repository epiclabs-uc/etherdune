// EtherFlow DHCP client demo
// Author: Javier Peletier <jm@friendev.com>
// Summary: Shows how to use DHCP in your project
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

/**
\file 
\brief Demonstrates the usage of the DHCP client
\details
Instantiates the DHCP service and requests an IP address.
The address will be renewed as long as the DHCP object instance
is alive.

See the DHCP documentation for more information.

\cond


*/
#include <ACross.h>
#include <FlowScanner.h>
#include <DHCP.h>


#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("DHCPTest");


static const uint8_t CS_PIN = 10; //Put here what pin you are using for your ENC28J60's chip select
static MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };

DHCP dhcp; //instantiate the DHCP service


void setup()
{

	Serial.begin(115200);
	ACross::init();

	Serial.println(F("EtherFlow DHCP sample"));
	Serial.print(F("Free RAM: ")); Serial.println(ACross::getFreeRam());
	Serial.println(F("Press any key to start..."));

	while (!Serial.available());

	net::localMAC = mymac;

	if (!net::begin(CS_PIN))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");

	Serial.println("Obtaining DHCP configuration...");

	if (!dhcp.dhcpSetup())
	{
		Serial.println(F("DHCP setup failed"));
		ACross::halt(1);
	}

	Serial.println(F("DHCP setup OK"));

	Serial.println(F("DHCP config:"));
	Serial.print(F("Local IP: "));
	Serial.println(net::localIP.toString());
	Serial.print(F("Network mask: "));
	Serial.println(net::netmask.toString());
	Serial.print(F("Gateway IP: "));
	Serial.println(net::gatewayIP.toString());
	Serial.print(F("DNS IP: "));
	Serial.println(net::dnsIP.toString());

	Serial.println(F("\nDHCP works!"));
	
}


void loop()
{
	net::loop(); //given the DHCP instance is alive, net::loop() will keep the lease alive.

}

/// \endcond