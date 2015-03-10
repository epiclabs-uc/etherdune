#include "HTTPServer.h"

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("HTTPServer");

void HTTPServer::onBodyReceived(uint16_t len, const byte* data){}
void HTTPServer::onHeaderReceived(const char* headerName, const char* headerValue){}
void HTTPServer::onBodyBegin(){}
void HTTPServer::onRequest(char* queryString){}
void HTTPServer::onRequestEnd(){}

HTTPServer::HTTPServer() :requestPattern(requestPatternString), headerPattern(headerPatternString)
{

}

void HTTPServer::listen(uint16_t port)
{
	localPort.setValue(80);
	TCPSocket::listen();
}

void HTTPServer::onReceive(uint16_t len, const byte* data)
{
	if (stage >= HTTP_SERVER_STAGE_RESPONSE)
		return;

	if (crlfcount == 4)
	{
		onBodyReceived(len, data);

		if (stage >= HTTP_SERVER_STAGE_RESPONSE)
			return;

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
					stage = HTTP_SERVER_STAGE_BODY;
					
					onBodyBegin();
					
					if (stage >= HTTP_SERVER_STAGE_RESPONSE)
						return;
					
					onBodyReceived(len, data);
					
					if (stage >= HTTP_SERVER_STAGE_RESPONSE)
						return;
					
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
			if (stage == HTTP_SERVER_STAGE_QUERY_STRING)
			{
				if (scanner.scan(c, queryString))
				{
					onRequest(queryString);

					if (stage >= HTTP_SERVER_STAGE_RESPONSE)
						return;

					crlfcount = 0;
					scanner.setPattern(headerPattern);
					stage = HTTP_SERVER_STAGE_HEADERS;
				}
			}
			else
			{
				if (c == ' ')
				{
					stage = HTTP_SERVER_STAGE_QUERY_STRING;
					continue;
				}
				httpMethod += c + 2;
			}

		}
	}

}

void HTTPServer::resetParser()
{
	scanner.setPattern(requestPattern);
	contentLength = 0;
	crlfcount = 0;
	stage = HTTP_SERVER_STAGE_INIT;
	httpMethod = 0;
}

void HTTPServer::onConnect()
{
	accept();
	ACTRACE("HTTP connection open");
	resetParser();
}

void HTTPServer::onClose()
{
	ACTRACE("HTTP connection closed by peer");
	close();

}

void HTTPServer::onTerminate()
{
	listen();
}

void HTTPServer::beginResponse(uint16_t statusCode, const String& message)
{
	stage = HTTP_SERVER_STAGE_RESPONSE;
	String code(statusCode);
	write(F("HTTP/1.1 % %\r\n"), &code, &message);
}
void HTTPServer::beginResponse_P(uint16_t statusCode, PGM_P message)
{
	beginResponse(statusCode, (__FlashStringHelper*)message);
}

void HTTPServer::writeHeader(const String& headerName, const String& headerValue)
{
	write(F("%:%\r\n"), &headerName, &headerValue);
}
void HTTPServer::writeHeader(PGM_P headerName, const String& headerValue)
{
	writeHeader((__FlashStringHelper*)headerName, headerValue);
}

void HTTPServer::writeContentTypeHeader(const String& contentType)
{
	writeHeader(HTTP_HEADER_CONTENT_TYPE, contentType);
}

void HTTPServer::writeContentTypeHeader_P(PGM_P contentType)
{
	String contentTypeStr((__FlashStringHelper*)contentType);
	writeContentTypeHeader(contentTypeStr);
}

void HTTPServer::beginResponseBody()
{
	stage = HTTP_SERVER_STAGE_RESPONSE_BODY;
	write(F("\r\n"));
}

void HTTPServer::endResponse()
{
	if (stage < HTTP_SERVER_STAGE_RESPONSE_BODY)
		beginResponseBody();

	push();
	close();

	stage = HTTP_SERVER_STAGE_RESPONSE_END;
}