#include "HTTPClient.h"


DEFINE_FLOWPATTERN(statusCodePattern, "HTTP/%*1d.%*1d %d%*99[^\r\n]\r\n");
DEFINE_FLOWPATTERN(headerNamePattern, "%30[^:]:");
DEFINE_FLOWPATTERN(headerValuePattern, "%*9[ ]%30[^\r\n]\r\n");
DEFINE_FLOWPATTERN(bodyBeginPattern, "\r\n\r\n");

void HTTPClient::onHeaderReceived(const char* header, const char* value) {}
void HTTPClient::onBodyReceived(uint16_t len, const byte* data) {}


HTTPClient::HTTPClient()
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
	headerScanner.setPattern(statusCodePattern);
	bodyBeginScanner.setPattern(bodyBeginPattern);

	statusCode = 0;
	write(F("GET % HTTP/1.1" "\r\n" "Accept:*" "/" "*" "\r\n" "Host:%\r\n\r\n"), &res, &host);




}

void HTTPClient::onReceive(uint16_t len, const byte* data)
{

	if (bodyBeginPattern.signaled)
	{
		onBodyReceived(len, data);
		return;
	}


	while (len--)
	{
		uint8_t c = *data++;

		if (statusCodePattern.signaled)
		{
			if (headerNamePattern.signaled)
			{
				if (headerScanner.scan(c, headerValue))
				{
					onHeaderReceived(headerName, headerValue);
					headerScanner.setPattern(headerNamePattern);
				}
			}
			else
			{

				if (headerScanner.scan(c, headerName))
				{
					headerScanner.setPattern(headerValuePattern);
				}
			}
		}
		else
		{
			if (headerScanner.scan(c, &statusCode))
			{
				headerScanner.setPattern(headerNamePattern);
			}

		}

		if (bodyBeginScanner.scan(c))
		{
			onBodyReceived(len, data);
			return;
		}

	}
}


void HTTPClient::onClose()
{
	close();
}

HTTPClient::~HTTPClient()
{
	onClose();
}