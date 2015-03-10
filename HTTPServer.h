#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <ACross.h>
#include "config.h"
#include "HTTPConstants.h"
#include "TCPSocket.h"
#include <FlowScanner.h>


#define requestPatternStringMaxLength(queryStringLength) "%" QUOTE(queryStringLength) "[^ ] HTTP/%*d.%*d\r\n"
#define headerPatternStringMaxLength(headerLength, valueLength) "%" QUOTE(headerLength) "[^:]:% %" QUOTE(valueLength) "[^\r]\r\n"


static const char requestPatternString[] PROGMEM = requestPatternStringMaxLength(HTTP_SERVER_QUERY_STRING_MAX_LENGTH);
static const char headerPatternString[] PROGMEM = headerPatternStringMaxLength(HTTP_SERVER_HEADER_NAME_MAX_LENGTH, HTTP_SERVER_HEADER_VALUE_MAX_LENGTH);


class HTTPServer :public TCPSocket
{
	union
	{
		struct
		{
			char queryString[HTTP_SERVER_QUERY_STRING_MAX_LENGTH + 1];
		};
		struct
		{
			char headerName[HTTP_SERVER_HEADER_NAME_MAX_LENGTH + 1];
			char headerValue[HTTP_SERVER_HEADER_VALUE_MAX_LENGTH + 1];
		};

	};

	static const uint8_t HTTP_SERVER_STAGE_INIT = 0;
	static const uint8_t HTTP_SERVER_STAGE_METHOD = 1;
	static const uint8_t HTTP_SERVER_STAGE_QUERY_STRING = 2;
	static const uint8_t HTTP_SERVER_STAGE_HEADERS = 3;
	static const uint8_t HTTP_SERVER_STAGE_BODY = 4;
	static const uint8_t HTTP_SERVER_STAGE_RESPONSE = 5;
	static const uint8_t HTTP_SERVER_STAGE_RESPONSE_BODY = 6;
	static const uint8_t HTTP_SERVER_STAGE_RESPONSE_END = 7;



	uint8_t stage;

	uint8_t crlfcount;

	FlowPattern requestPattern;
	FlowPattern headerPattern;

	FlowScanner scanner;

public:
	uint8_t httpMethod;
	uint16_t contentLength;

protected:
	void resetParser();

public:

	virtual void onBodyReceived(uint16_t len, const byte* data);
	virtual void onHeaderReceived(const char* headerName, const char* headerValue);
	virtual void onBodyBegin();
	virtual void onRequest(char* queryString);
	virtual void onRequestEnd();

	HTTPServer();

	void listen(uint16_t port = 80);
	void onReceive(uint16_t len, const byte* data);
	void onConnect();
	void onClose();
	void onTerminate();
	void beginResponse(uint16_t statusCode, const String& message = "");
	void beginResponse_P(uint16_t statusCode, PGM_P message);
	void writeHeader(const String& headerName, const String& headerValue);
	void writeHeader(PGM_P headerName, const String& headerValue);
	void writeContentTypeHeader(const String& contentType);
	void writeContentTypeHeader_P(PGM_P contentType);
	void beginResponseBody();
	void endResponse();

};







#endif