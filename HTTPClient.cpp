#include "HTTPClient.h"
#include "DNS.h"


void HTTPClient::onResponseReceived() {}
void HTTPClient::onResponseEnd() {}
void HTTPClient::onHeaderReceived(uint16_t len, const byte* data) {}
void HTTPClient::onBodyReceived(uint16_t len, const byte* data) {}


HTTPClient::HTTPClient() :
	statusCodePattern(statusCodePatternString),
	bodyBeginPattern(bodyBeginPatternString),
	DNSid(0)
{
	
}

void HTTPClient::request(const String& hostName, const String& resource, uint16_t port )
{
	remotePort.setValue(port);
	DNSid = DNS.resolve(hostName.c_str());
	host = hostName;
	res = resource;
}


void HTTPClient::onDNSResolve(uint16_t identification, const IPAddress& ip)
{
	if (identification == DNSid)
	{
		DNSid = 0;
		remoteIP = ip;
		connect();
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