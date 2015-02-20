#include "HTTPClient.h"



void HTTPClient::onResponseReceived() {}
void HTTPClient::onResponseEnd() {}
void HTTPClient::onHeaderReceived(uint16_t len, const byte* data) {}
void HTTPClient::onBodyReceived(uint16_t len, const byte* data) {}


HTTPClient::HTTPClient() :statusCodePattern(statusCodePatternString), bodyBeginPattern(bodyBeginPatternString)
{
	
}

void HTTPClient::request(const String& hostName, const String& resource)
{

	host = hostName;
	res = resource;
	connect();
}


void HTTPClient::onConnect()
{
	scanner.setPattern(statusCodePattern);
	statusCode = 0;
	contentLength = 0;
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