#include <ACross.h>
#include <Checksum.h>
#include <TCPSocket.h>

#include <inet.h>
#include <ENC28J60.h>
#include <DNS.h>
#include <FlowScanner.h>
#include <HTTPServer.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("HTTPServerDemo");


MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP = { 192, 168, 1, 33 };
IPAddress_P netmask = { 255, 255, 255, 0 };

DNSClient net::DNS;

#include <HTTPServer.h>



class HTTPServerTest : public HTTPServer
{
public:
	static const uint8_t ACTION_DIGITALREAD = 1;
	static const uint8_t ACTION_ANALOGREAD = 2;

	void onRequest(char* queryString)
	{
		ACTRACE("Requested URL:%s", queryString);

		if (strcmp_P(queryString, PSTR("/")) == 0)
		{
			beginResponse_P(HTTP_RESPONSE_OK, HTTP_RESPONSE_OK_STR);
			writeContentTypeHeader_P(CONTENT_TYPE_TEXT_HTML);
			beginResponseBody();
			String title(F("Arduino Pin Monitor"));

			write(F("<html><head><title>%</title></head><body><h1>%</h1><h2>Select pin to monitor:</h2><h3>digital</h3><ul>"),&title,&title);

			for (uint8_t pin = 2; pin < 13; pin++)
			{
				String pinStr(pin);
				write(F("<li><a href=\"/digitalRead/%\">Pin %</a></li>"), &pinStr, &pinStr);
			}
			
			write(F("</ul><h3>Analog</h3><ul>"));
			
			for (uint8_t pin = A0; pin <= A7; pin++)
			{
				String pinStr(pin);
				String APinStr(pin-A0);
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
				uint8_t action;
				uint8_t pin;
				char* p;
				p = strtok(queryString, "/");


				if (p != NULL)
				{
					if (strcmp_P(p, PSTR("digitalRead")) == 0)
						action = ACTION_DIGITALREAD;
					else
						if (strcmp_P(p, PSTR("analogRead")) == 0)
							action = ACTION_ANALOGREAD;

				}
				p = strtok(NULL, "/");
				if (p != NULL)
				{
					pin = atoi(p);
				}


				//for illustrative purposes, verify the client used a GET request
				if (httpMethod == HTTP_METHOD_GET)
				{

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
					beginResponse(HTTP_RESPONSE_METHOD_NOT_ALLOWED, HTTP_RESPONSE_METHOD_NOT_ALLOWED_STR);
					writeHeader(F("Allow"), F("GET"));
					beginResponseBody();
					write(F("Use a GET request"));
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



}server;




void setup()
{

	Serial.begin(115200);
	ACross::init();
#ifdef ACROSS_ARDUINO
	ACross::printf_serial_init();
#endif

	printf(PSTR("HTTP Server EtherFlow sample\n"));
	printf(PSTR("Press any key to start...\n"));

	while (!Serial.available());

	net::localMAC = mymac;
	net::localIP = myIP;
	net::gatewayIP = gatewayIP;
	net::netmask = netmask;

	if (!net::begin(10))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");


	server.listen();

	Serial.print("HTTP Server listening on ");
	Serial.print(net::localIP.toString());
	Serial.print(":");
	Serial.println(server.localPort.getValue());
}

void loop()
{
	net::loop();

}

