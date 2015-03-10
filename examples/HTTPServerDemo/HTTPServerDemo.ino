#include <ACross.h>
#include <Checksum.h>
#include <TCPSocket.h>

#include <inet.h>
#include <EtherFlow.h>
#include <DNS.h>
#include <FlowScanner.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("PingTest");


MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP = { 192, 168, 1, 33 };
IPAddress_P netmask = { 255, 255, 255, 0 };

#include <HTTPConstants.h>

static const char requestPatternString[] PROGMEM = "%41[^ ] HTTP/%*d.%*d\r\n";
static const char headerPatternString[] PROGMEM = "%20[^:]:% %20[^\r]\r\n";





class HTTPServer :protected TCPSocket
{
	union
	{
		struct
		{
			char queryString[42];
		};
		struct
		{
			char headerName[21];
			char headerValue[21];
		};

	};
	
	bool httpMethodFound;
	
	uint8_t crlfcount;

	FlowPattern requestPattern;
	FlowPattern headerPattern;

	FlowScanner scanner;

public:
	uint8_t httpMethod;
	uint16_t contentLength;

public:
	HTTPServer() :requestPattern(requestPatternString), headerPattern(headerPatternString)
	{

	}


	void listen(uint16_t port = 80)
	{
		localPort.setValue(80);
		TCPSocket::listen();
	}

	void onReceive(uint16_t len, const byte* data)
	{
		if (crlfcount==4)
		{
			onBodyReceived(len, data);
			contentLength -= len;
			if (contentLength == 0)
			{
				onRequestEnd();
			}
			return;
		}

		while (len--)
		{
			uint8_t c = *data++;

			if (requestPattern.signaled)
			{
				if (c == '\r' || c == '\n')
				{
					crlfcount++;
					if (crlfcount == 4)
					{
						onBodyBegin();
						onBodyReceived(len, data);
						contentLength -= len;
						if (contentLength == 0)
							onRequestEnd();

						return;
					}
				}
				else
					crlfcount = 0;

				if (scanner.scan(c, headerName, headerValue))
				{
					if (strcasecmp_P(headerName, HTTP_HEADER_CONTENT_LENGTH))
					{
						contentLength = atoi(headerValue);
					}
					onHeaderReceived(headerName, headerValue);
					scanner.reset();
					
				}
	
			}
			else
			{
				if (httpMethodFound)
				{
					if (scanner.scan(c, queryString))
					{
						onRequest(queryString);
						crlfcount = 0;
						scanner.setPattern(headerPattern);
					}
				}
				else
				{
					if (c == ' ')
					{
						httpMethodFound = true;
						continue;
					}
					httpMethod += c + 2;
				}

			}
		}

	}

	void onConnect()
	{
		accept();
		ACTRACE("HTTP connection open");
		scanner.setPattern(requestPattern);
		contentLength = 0;
		crlfcount = 0;
		httpMethodFound = 0;
		httpMethod = 0;

	}

	void onClose()
	{
		ACTRACE("HTTP connection closed by peer");
		close();

	}

	void onTerminate()
	{
		listen();
	}

	void beginResponse(uint16_t statusCode, const String& message = "")
	{
		String code(statusCode);
		write(F("HTTP/1.1 % %\r\n"), &code, &message);
	}
	void beginResponse_P(uint16_t statusCode, PGM_P message)
	{
		beginResponse(statusCode, (__FlashStringHelper*) message);
	}

	void writeHeader(const String& headerName, const String& headerValue)
	{
		write(F("%:%\r\n"), &headerName, &headerValue);
	}
	void writeHeader(PGM_P headerName, const String& headerValue)
	{
		writeHeader((__FlashStringHelper*)headerName, headerValue);
	}

	void writeContentTypeHeader(const String& contentType)
	{
		writeHeader(HTTP_HEADER_CONTENT_TYPE, contentType);
	}

	void writeContentTypeHeader_P(PGM_P contentType)
	{
		String contentTypeStr((__FlashStringHelper*)contentType);
		writeContentTypeHeader(contentTypeStr);
	}

	void beginResponseBody()
	{
		write(F("\r\n"));
	}

	void endResponse()
	{
		push();
		close();
	}


	virtual void onBodyReceived(uint16_t len, const byte* data){}
	virtual void onHeaderReceived(const char* headerName, const char* headerValue){}
	virtual void onBodyBegin(){}
	virtual void onRequest(char* queryString){}
	virtual void onRequestEnd(){}

};



class HTTPServerTest : public HTTPServer
{
public:
	static const uint8_t ACTION_DIGITALREAD = 1;
	static const uint8_t ACTION_ANALOGREAD = 2;

	uint8_t action;
	uint8_t pin;

	void start()
	{
		listen();
	}

	void onRequest(char* queryString)
	{
		ACTRACE("Requested URL:%s", queryString);

		char* p;
		p = strtok(queryString, "/");
		

		if (p != NULL)
		{
			if (strcmp_P(p, PSTR("digitalRead"))==0)
				action = ACTION_DIGITALREAD;
			else
				if (strcmp_P(p, PSTR("analogRead"))==0)
					action = ACTION_ANALOGREAD;

		}
		p = strtok(NULL, "/");
		if (p != NULL)
		{
			pin = atoi(p);
		}

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
		
		//for illustrative purpose, verify the client used a GET request
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

		endResponse();


		
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


	server.start();
}

void loop()
{
	NetworkService::loop();

}

