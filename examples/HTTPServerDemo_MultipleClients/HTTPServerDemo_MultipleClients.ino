// EtherFlow HTTP Server demo supporting simultaneous connections
// Author: Javier Peletier <jm@friendev.com>
// Summary: Demonstrates how to build a simple web server that is able to serve multiple clients simultaneously
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
/// \details Demonstrates how to build a simple web server that is able to serve multiple clients simultaneously
/// allowing users to monitor the status of various hardware pins
/// See the HTTPServer and TCPListener class documentation for more information.
/// \cond

#include <ACross.h>
#include <Checksum.h>
#include <TCPSocket.h>

#include <inet.h>
#include <ENC28J60.h>
#include <FlowScanner.h>
#include <HTTPServer.h>
#include <TCPListener.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("HTTPServerDemo_MultipleClients");


static const uint8_t CS_PIN = 10; //Put here what pin you are using for your ENC28J60's chip select
static MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };
static IPAddress_P gatewayIP = { 192, 168, 1, 1 };
static IPAddress_P myIP = { 192, 168, 1, 33 };
static IPAddress_P netmask = { 255, 255, 255, 0 };

static const uint8_t MAX_CLIENTS = 2; // maximum number of simultaneous HTTP clients

class HTTPServerTestMulti : public HTTPServer
{
public:
	static const uint8_t ACTION_DIGITALREAD = 1;
	static const uint8_t ACTION_ANALOGREAD = 2;

	void onRequest(char* queryString)
	{
		ACTRACE("Requested URL:%s", queryString);

		//for illustrative purposes, enforce that only a GET method is used to access this server
		if (httpMethod != HTTP_METHOD_GET)
		{
			beginResponse(HTTP_RESPONSE_METHOD_NOT_ALLOWED, HTTP_RESPONSE_METHOD_NOT_ALLOWED_STR);
			writeHeader(F("Allow"), F("GET"));
			beginResponseBody();
			write(F("Use a GET request"));

		}
		else
		{

			if (strcmp_P(queryString, PSTR("/")) == 0)
			{
				beginResponse_P(HTTP_RESPONSE_OK, HTTP_RESPONSE_OK_STR);
				writeContentTypeHeader_P(CONTENT_TYPE_TEXT_HTML);
				beginResponseBody();
				String title(F("Arduino Pin Monitor"));

				write(F("<html><head><title>%</title></head><body><h1>%</h1><h2>Select pin to monitor:</h2><h3>digital</h3><ul>"), &title, &title);

				for (uint8_t pin = 2; pin < 13; pin++)
				{
					String pinStr(pin);
					write(F("<li><a href=\"/digitalRead/%\">Pin %</a></li>"), &pinStr, &pinStr);
				}

				write(F("</ul><h3>Analog</h3><ul>"));

				for (uint8_t pin = A0; pin <= A7; pin++)
				{
					String pinStr(pin);
					String APinStr(pin - A0);
					write(F("<li><a href=\"/analogRead/%\">Pin A%</a></li>"), &pinStr, &APinStr);

				}
				write(F("</ul></html>"));

			}
			else
			{
				if (strcmp_P(queryString, PSTR("/favicon.ico")) == 0)
				{
					beginResponse_P(HTTP_RESPONSE_FOUND, HTTP_RESPONSE_FOUND_STR);
					writeHeader(HTTP_HEADER_LOCATION, F("http://www.iconj.com/ico/c/a/capa77m3l6.ico"));
				}
				else
				{
					uint8_t action = 0;
					uint8_t pin;
					char* p;
					p = strtok(queryString, "/");


					if (p != NULL)
					{
						if (strcmp_P(p, PSTR("digitalRead")) == 0)
							action = ACTION_DIGITALREAD;
						else
						{
							if (strcmp_P(p, PSTR("analogRead")) == 0)
								action = ACTION_ANALOGREAD;
						}
					}

					if (action != 0)
					{
						p = strtok(NULL, "/");
						if (p != NULL)
						{
							pin = atoi(p);
						}

						beginResponse_P(HTTP_RESPONSE_OK, HTTP_RESPONSE_OK_STR);
						writeContentTypeHeader_P(CONTENT_TYPE_TEXT_HTML);
						beginResponseBody();
						String strPin(pin);
						uint16_t val = (action == ACTION_ANALOGREAD) ? analogRead(pin) : digitalRead(pin);
						String strVal(val);

						write(F("<html><head><meta http-equiv=\"refresh\" content=\"10\" /></head><body><h1>pin%=%</h1></body></html>"), &strPin, &strVal);
					}
					else
					{
						beginResponse_P(HTTP_RESPONSE_NOT_FOUND, HTTP_RESPONSE_NOT_FOUND_STR);
						beginResponseBody();
						write(F("<html><body><h1>404 not found!</h1></body></html>"));
					}
				}
			}
		}

		endResponse();
	}

	void onHeaderReceived(const char* headerName, const char* headerValue)
	{
		ACTRACE("HTTP Header: %s = '%s'", headerName, headerValue);
	}

	void onBodyBegin()
	{
		ACTRACE("onBodyBegin: Content-Length = %d", contentLength);
	}

	void onRequestEnd()
	{
		ACTRACE("Request ended");
	}





};


TCPListener<HTTPServerTestMulti, MAX_CLIENTS> server;


void setup()
{

	Serial.begin(115200);
	ACross::init();
#ifdef ACROSS_ARDUINO
	ACross::printf_serial_init();
#endif

	printf(PSTR("Multiple client HTTP Server EtherFlow sample\n"));
	Serial.print(F("Free RAM: ")); Serial.println(ACross::getFreeRam());
	printf(PSTR("Press any key to start...\n"));

	while (!Serial.available());

	net::localMAC = mymac;
	net::localIP = myIP;
	net::gatewayIP = gatewayIP;
	net::netmask = netmask;

	if (!net::begin(CS_PIN))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");


	server.listen(80);

	Serial.print("HTTP Server listening on ");
	Serial.print(net::localIP.toString());
	Serial.print(":");
	Serial.println(server.localPort);
}

void loop()
{
	net::loop();

}

/// \endcond