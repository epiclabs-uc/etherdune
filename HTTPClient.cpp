#include "HTTPClient.h"
#include "DNS.h"

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("HTTPClient");


void HTTPClient::onResponseReceived() {}
void HTTPClient::onResponseEnd() {}
void HTTPClient::onHeaderReceived(uint16_t len, const byte* data) {}
void HTTPClient::onBodyReceived(uint16_t len, const byte* data) {}
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
		DNSid = DNS.resolve(hostName.c_str());
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