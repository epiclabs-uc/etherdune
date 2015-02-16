#include "HTTPClient.h"


DEFINE_FLOWPATTERN(statusCodePattern, "HTTP/%*1d.%*1d %d%*99[^\r\n]\r\n");
DEFINE_FLOWPATTERN(headerPattern, "%30[^:]:%30[^\r\n]\r\n");
DEFINE_FLOWPATTERN(bodyBeginPattern, "\r\n\r\n");

void HTTPClient::onHeaderReceived(char* header, char* value) {}
void HTTPClient::onBodyReceived(uint16_t len, const byte* data) {}


HTTPClient::HTTPClient()
{
	headerName = NULL;
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

	if (!headerName)
	{
		headerName = new char[30];
		headerValue = new char[30];
	}

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
			if (headerScanner.scan(c, headerName, headerValue))
			{
				onHeaderReceived(headerName, headerValue);
				headerScanner.reset();
			}
		}
		else
		{
			if (headerScanner.scan(c, &statusCode))
			{
				headerScanner.setPattern(headerPattern);
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

	if (headerName)
	{
		delete headerName;
		delete headerValue;
		headerName = NULL;
	}
}

HTTPClient::~HTTPClient()
{
	onClose();
}