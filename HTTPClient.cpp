// EtherFlow HTTP Client class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Provides an easy way to query a web server
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

#include "HTTPClient.h"
#include "DNS.h"

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("HTTPClient");


/// <summary>
/// Called immediately after the first line is received and there is a status code, e.g. `200 OK`.
/// The HTTP status code is stored in the \ref statusCode member variable.
/// </summary>
void HTTPClient::onResponseReceived() {}
/// <summary>
/// Called after all the body has been received
/// </summary>
void HTTPClient::onResponseEnd() {}
/// <summary>
/// Called once for each fragment of the header portion of the response
/// </summary>
/// <param name="len">length of the data byte array containing the header</param>
/// <param name="data">pointer to the buffer containing the header data</param>
void HTTPClient::onHeaderReceived(uint16_t len, const byte* data) {}
/// <summary>
/// Called once for each fragment of the body that is received.
/// </summary>
/// <param name="len">length of the buffer that contains the body portion received</param>
/// <param name="data">pointer to the buffer containing the body data</param>
void HTTPClient::onBodyReceived(uint16_t len, const byte* data) {}
/// <summary>
/// Called when all HTTP headers have been received and the body of the response is about to arrive.
/// This call gives an opportunity to configure the scanner (FlowScanner) or other parser to process the
/// body as it arrives.
/// </summary>
void HTTPClient::onBodyBegin(){};


HTTPClient::HTTPClient() :
	statusCodePattern(statusCodePatternString),
	bodyBeginPattern(bodyBeginPatternString),
	DNSid(0)
{
	remoteIP.u = 0;
	remotePort.setValue(80);
}

void HTTPClient::request(const String& hostName, const String& resource, uint16_t port )
{
	remotePort.setValue(port);

	host = hostName;
	res = resource;

	if (remoteIP.u == 0)
		DNSid = DNS().resolve(hostName.c_str());
	else
		connect();
}


void HTTPClient::onDNSResolve(uint8_t status, uint16_t identification, const IPAddress& ip)
{
	if (identification == DNSid)
	{
		if (status == 0)
		{
			DNSid = 0;
			remoteIP = ip;
			connect();
		}
		else
		{
			ACERROR("Could not resolve hostname");
		}
	}
}


void HTTPClient::onConnect()
{
	scanner.setPattern(statusCodePattern);
	statusCode = 0;
	bodyBeginPattern.signaled = false;

	write(F("GET % HTTP/1.1" "\r\n" "Accept:*" "/" "*" "\r\n" "Host:%\r\n\r\n"), &res, &host);
}

void HTTPClient::onReceive(uint16_t len, const byte* data)
{

	if (bodyBeginPattern.signaled)
	{
		onBodyReceived(len, data);
		return;
	}

	const byte* headerStart = data;

	while (len--)
	{
		uint8_t c = *data++;


		if (statusCodePattern.signaled)
		{
			if (scanner.scan(c))
			{
				onHeaderReceived(data - headerStart, headerStart);
				onBodyBegin();
				onBodyReceived(len, data);
				return;
			}
		}
		else
		{
			if (scanner.scan(c, &statusCode))
			{
				onResponseReceived();
				scanner.setPattern(bodyBeginPattern);
				headerStart = data;
			}

		}
	}

	if (statusCodePattern.signaled)
		onHeaderReceived(data - headerStart, headerStart);

}


void HTTPClient::onClose()
{
	close();
	onResponseEnd();
}

HTTPClient::~HTTPClient()
{
	onClose();
}