#include "HTTPClient.h"


void HTTPClient::request(const String& hostName, const String& resource)
{
	host = hostName;
	res = resource;
	connect();
}


void HTTPClient::onConnect()
{
	//write(F("GET "));
	//write(res);
	//write(F(" HTTP/1.1" "\r\n" "Accept:*" "/" "*" "\r\n" "Host:"));
	//write(host);
	//write(F("\r\n\r\n"));
	
	write(F("GET % HTTP/1.1" "\r\n" "Accept:*" "/" "*" "\r\n" "Host:%\r\n\r\n"), &res, &host);

	


//15788

}

void HTTPClient::onReceive(uint16_t len, const byte* data)
{

}