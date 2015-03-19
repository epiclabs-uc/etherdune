// EtherFlow UDP Client demo: NTP client
// Author: Javier Peletier <jm@friendev.com>
// Summary: Demonstrates how to build a simple UDP client application
// that syncs with an NTP server.
//
// Copyright (c) 2015 All Rights Reserved, http://friendev.com
//
// Credits: Some time calculation code inspired by Paul Stoffregen's Time.cpp 
//         https://github.com/PaulStoffregen/Time/blob/master/Time.cpp
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
\brief Demonstrates how to build a simple UDP client application
\details
Connects to a NTP server periodically to get the time

Also, and for fun, uses millis() to keep the obtained time updated

See the UDPClient documentation for more information.

\cond


*/


#include <ACross.h>
#include <Checksum.h>
#include <TCPSocket.h>
#include <UDPSocket.h>

#include <inet.h>
#include <ENC28J60.h>
#include <FlowScanner.h>
#include <HTTPClient.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("UDPClient");


static const uint8_t CS_PIN = 10; //Put here what pin you are using for your ENC28J60's chip select
static MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };
static IPAddress_P gatewayIP = { 192, 168, 1, 1 };
static IPAddress_P myIP  = { 192, 168, 1, 33 };
static IPAddress_P netmask  = { 255, 255, 255, 0 };

static IPAddress_P NTPServerIP = { 206, 246, 122, 250 }; // nist1-pa.ustiming.org. alternatives: http://tf.nist.gov/tf-cgi/servers.cgi


// NTP message format struct definitions
//
/* 
For this simple demo it wouldn't have been necessary to be so formal
defining the structs, etc, but it is a good example of how
Socket::write() works, allowing you to just build the struct
and put it on the wire.

See RFC1305 https://tools.ietf.org/html/rfc1305
*/

// NTP Short timestamp
struct NTPShort
{	
	nint16_t seconds;
	nint16_t fraction;

};

// NTP Timestamp definition
struct NTPTimestamp
{
	nint32_t seconds;
	nint32_t fraction;

	void zero()
	{
		seconds.zero();
		fraction.zero();
	}
};

// Structure of an NTP message according to the RFC.
struct NTPMessage
{
	uint8_t mode : 3;
	uint8_t versionNumber : 3;
	uint8_t leapIndicator : 2;
	uint8_t stratum;
	uint8_t pollingInterval;
	uint8_t precision;
	NTPShort rootDelay;
	NTPShort rootDispersion;
	union
	{
		uint32_t referenceId;
		char referenceStr[4];
	};
	NTPTimestamp reference;
	NTPTimestamp origin;
	NTPTimestamp receive;
	NTPTimestamp transmit;
	
};


class NTPClient : UDPSocket
{
	static const int timeZone = 1; //Time offset from GMT
	static const uint16_t NTP_UPDATE_INTERVAL_TICKS = NTICKS(10 * 60 * 1000); //sync every 10 minutes with NTP server


	static const unsigned long secs1900_1970 = (17UL * 366UL + (70UL - 17UL) * 365UL) * 24UL * 60UL * 60UL; // Seconds between 1970 and 1800. 17 leap years.
	

private: 
	uint32_t millis_offset;
	uint32_t epoch;
	uint16_t updateTimer;

	bool isLeapYear(uint16_t y)
	{
		return !((1970 + y) % 4) && (((1970 + y) % 100) || !((1970 + y) % 400));
	}

public:

	uint16_t year;
	uint8_t month;
	uint8_t weekDay;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;


	void start()
	{
		remotePort = 123; //NTP UDP port
		remoteIP = NTPServerIP;
		updateTimer = 0;
		epoch = 0;

	}

	void syncTime()
	{
		NTPMessage msg;

		msg.leapIndicator = 3;
		msg.versionNumber = 3;
		msg.mode = 3;
		msg.stratum = 0;
		msg.pollingInterval = 17;
		msg.precision = 0xFA;
		msg.rootDelay.fraction.zero();
		msg.rootDelay.seconds.zero();
		msg.rootDispersion.seconds = 1;
		msg.rootDispersion.fraction = 0;
		msg.referenceId = 0;
		msg.reference.zero();
		msg.reference.fraction = 0;
		msg.origin.zero();
		msg.receive.zero();
		msg.transmit.zero();

		write(msg);
		send();
	}


	void onReceive(uint16_t len)
	{
		ACASSERT(len == 48, "Incorrect frame length %d", len);
		
		millis_offset = millis();

		NTPMessage& msg = *(NTPMessage*)packet.udpData;

		epoch = msg.transmit.seconds - secs1900_1970;

	}

	void getTime()
	{
		//inspired by Paul Stoffregen's Time.cpp https://github.com/PaulStoffregen/Time/blob/master/Time.cpp
		const uint8_t daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

		uint32_t ep = epoch + (millis() - millis_offset) / 1000;

		second = ep % 60;
		ep /= 60;
		minute = ep % 60;
		ep /= 60;
		hour = ((ep % 24) + timeZone) % 24;
		ep /= 24;
		weekDay = ((ep + 4) % 7);
		year = 0;
		uint32_t days = 0;
		
		while ((uint32_t)(days += (isLeapYear(year) ? 366 : 365)) <= ep)
		{
			year++;
		}

		days -= isLeapYear(year) ? 366 : 365;
		ep -= days;
		days = 0;
		month = 0;

		uint16_t d = ep;
		uint8_t dm;

		for (uint8_t m = 0; m < 12; m++)
		{
			dm = (m == 1) ? (isLeapYear(year) ? 29 : 28) : daysInMonth[m];

			if (d > dm)
			{
				d -= dm;
			}
			else
			{
				month = m + 1;
				day = d + 1;
				break;
			}
		}

		year += 1970;

	}

	bool inSync()
	{
		return epoch != 0;
	}

	void tick()
	{

		if (updateTimer == 0 || !inSync())
		{
			updateTimer = NTP_UPDATE_INTERVAL_TICKS;
			syncTime();
		}
		else
		{
			updateTimer--;
		}


		UDPSocket::tick();
	}


}ntp;


uint32_t timer;

void setup()
{	

	Serial.begin(115200);
	ACross::init();	
#ifdef ACROSS_ARDUINO
	ACross::printf_serial_init();
#endif

	printf(PSTR("NTP/UDP Client Etherflow sample\n"));
	Serial.print(F("Free RAM: ")); Serial.println(ACross::getFreeRam());
	Serial.println(F("Press any key to start..."));

	while (!Serial.available());


	net::localIP = myIP;
	net::localMAC = mymac;
	net::gatewayIP = gatewayIP;
	net::netmask = netmask;


	if (!net::begin(CS_PIN))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");

	ntp.start();

	timer = millis();
}



void loop()
{
	net::loop();


	if ((int32_t)(millis() - timer) >= 0)
	{
		timer += 1000;

		if (ntp.inSync())
		{

			ntp.getTime();

			printf_P(PSTR("Current date/time: %.4d-%.2d-%.2d %.2d:%.2d:%.2d\n"), ntp.year, ntp.month, ntp.day, ntp.hour, ntp.minute, ntp.second);

		}
		else
		{
			Serial.println(F("Waiting for time sync..."));
		}

	}

}


/// \endcond