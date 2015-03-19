// EtherFlow DNS and TCP Daytime client demo
// Author: Javier Peletier <jm@friendev.com>
// Summary: Demonstrates how to build a simple TCP client application
// and how to resolve a host name to an IP address.
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
\brief Demonstrates how to build a simple TCP client application
\details
Connects to a DAYTIME server and prints out the current time

See the TCPSocket documentation for more information.

\cond


*/


#include <ACross.h>
#include <FlowScanner.h>
#include <TCPSocket.h>
#include <DNS.h>


#define AC_LOGLEVEL 5
#include <ACLog.h>
ACROSS_MODULE("TCPClient Demo");


static const uint8_t CS_PIN = 10; //Put here what pin you are using for your ENC28J60's chip select
static MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };
static IPAddress_P gatewayIP = { 192, 168, 1, 1 };
static IPAddress_P myIP = { 192, 168, 1, 33 };
static IPAddress_P netmask = { 255, 255, 255, 0 };



class MyProtocol : public TCPSocket
{
	uint16_t DNSid;
public:

	void start()
	{
		remotePort = 13;

		DNSid = net::DNS().resolve("ntp-nist.ldsbc.edu");
		//alternatives here: http://www.jumpjet.info/Offbeat-Internet/Public/Daytime/url.htm
	}

	void onConnect()
	{
		ACTRACE("on connect");
		Serial.print(F("Time is:"));
	}

	void onClose()
	{
		ACTRACE("on close");
		close(); //properly close the connection.
	}

	void onReceive(uint16_t len, const byte* data)
	{
		ACTRACE("onReceive: %d bytes", len);
		Serial.write(data, len);
		//time info comes in this format
		//http://www.nist.gov/pml/div688/grp40/its.cfm
		//this code can be improved by parsing the received string to sync Arduino's own timer, for example
	}


	void onDNSResolve(uint8_t status, uint16_t identification, const IPAddress& resolvedIP)
	{
		
		if (identification == DNSid && status==0)
		{
			ACINFO("resolved. IP=%d.%d.%d.%d", resolvedIP.b[0], resolvedIP.b[1], resolvedIP.b[2], resolvedIP.b[3]);
			remoteIP = resolvedIP;
			connect();
		}
	}


} sck;


void setup()
{

	Serial.begin(115200);
	ACross::init();

	Serial.println(F("TCP Client EtherFlow example"));
	Serial.print(F("Free RAM: ")); Serial.println(ACross::getFreeRam());
	Serial.println(F("Press any key to start..."));

	while (!Serial.available());


	net::localIP = myIP;
	net::localMAC = mymac;
	net::gatewayIP = gatewayIP;
	net::netmask = netmask;
	net::dnsIP = IPADDR_P(8, 8, 8, 8);

	if (!net::begin(CS_PIN))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");

	

	Serial.println(F("connecting..."));
	sck.start();

}



void loop()
{
	net::loop();
}

/// \endcond