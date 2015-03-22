// EtherDune HTTP Server class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Provides an easy way to build a web server
//
// Copyright (c) 2015 All Rights Reserved, http://friendev.com
//
// This source is subject to the GPLv2 license.
// Please see the License.txt file for more information.
// All other rights reserved.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.

#include "HTTPServer.h"

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("HTTPServer");

#define requestPatternStringMaxLength(queryStringLength) "%" QUOTE(queryStringLength) "[^ ] HTTP/%*d.%*d\r\n"
#define headerPatternStringMaxLength(headerLength, valueLength) "%" QUOTE(headerLength) "[^:]:% %" QUOTE(valueLength) "[^\r]\r\n"

static const char requestPatternString[] PROGMEM = requestPatternStringMaxLength(HTTP_SERVER_QUERY_STRING_MAX_LENGTH);
static const char headerPatternString[] PROGMEM = headerPatternStringMaxLength(HTTP_SERVER_HEADER_NAME_MAX_LENGTH, HTTP_SERVER_HEADER_VALUE_MAX_LENGTH);


/// <summary>
/// Called once for each fragment of the body that is received.
/// </summary>
/// <param name="len">length of the buffer that contains the body portion received</param>
/// <param name="data">pointer to the buffer containing the body data</param>
void HTTPServer::onBodyReceived(uint16_t len, const byte* data){}
/// <summary>
/// Called once for each header in the request, as they arrive.
/// </summary>
/// <param name="headerName">pointer to string containing the header name, e.g. `Accept`</param>
/// <param name="headerValue">pointer to string containing the header value, e.g. `text/html`</param>
/// <remarks>The maximum length of `headerName` and `headerValue` is limited to 
/// \ref HTTP_SERVER_HEADER_NAME_MAX_LENGTH and \ref HTTP_SERVER_HEADER_VALUE_MAX_LENGTH respectively.
/// see \ref HTTPServerConfig for more information </remarks>
void HTTPServer::onHeaderReceived(const char* headerName, const char* headerValue){}
/// <summary>
/// Called when all HTTP headers have been received and the body of the request is about to arrive.
/// This call gives an opportunity to configure scanner FlowScanner HTTPServer::scanner or other parser to process the
/// body as it arrives.
/// </summary>
void HTTPServer::onBodyBegin(){}
/// <summary>
/// Called immediately after the first line that contains the HTTP method and query string is received
/// </summary>
/// <param name="queryString">pointer to the request's query string. The length of this string is limited to 
/// \ref HTTP_SERVER_QUERY_STRING_MAX_LENGTH configuration value. see \ref HTTPServerConfig for more information.</param>
void HTTPServer::onRequest(char* queryString){}
/// <summary>
/// Called after all the body has been received
/// </summary>
void HTTPServer::onRequestEnd(){}

HTTPServer::HTTPServer() :requestPattern(requestPatternString), headerPattern(headerPatternString)
{

}

/// <summary>
/// Starts listening on the specified TCP port.
/// </summary>
/// <param name="port">TCP port to listen on</param>
void HTTPServer::listen(uint16_t port)
{
	localPort = port;
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

void HTTPServer::onConnectRequest()
{
	accept(); //accept any connection request
}
void HTTPServer::onConnect()
{
	ACTRACE("HTTP connection open");
	resetParser();
}

void HTTPServer::onClose()
{
	ACTRACE("HTTP connection closed by peer");
	close();

}

/// <summary>
/// Sends out the response line, e.g. `HTTP/1.1 200 OK`
/// </summary>
/// <param name="statusCode">HTTP status code to send, see \ref HTTPConstants_ResponseCodes for a list of possible values</param>
/// <param name="message">HTTP message to send as status description, as a String or regular C string</param>
void HTTPServer::beginResponse(uint16_t statusCode, const String& message)
{
	stage = HTTP_SERVER_STAGE_RESPONSE;
	String code(statusCode);
	write(F("HTTP/1.1 % %\r\n"), &code, &message);
}
/// <summary>
/// Sends out the response line, e.g. `HTTP/1.1 404 Not Found`
/// </summary>
/// <param name="statusCode">HTTP status code to send, see \ref HTTPConstants_ResponseCodes for a list of possible values</param>
/// <param name="message">HTTP message to send as status description, as a PROGMEM string</param>
void HTTPServer::beginResponse_P(uint16_t statusCode, PGM_P message)
{
	beginResponse(statusCode, (__FlashStringHelper*)message);
}

/// <summary>
/// Writes an HTTP header to the ongoing response stream. Use only after beginResponse() has been called to initiate the HTTP
/// response.
/// </summary>
/// <param name="headerName">Name of the header, e.g. `Last-Modified`, as a String object or regular null-terminated C string</param>
/// <param name="headerValue">The header value, e.g. `Sat, 14 Feb 2015 01:55:08 GMT`</param>
void HTTPServer::writeHeader(const String& headerName, const String& headerValue)
{
	write(F("%:%\r\n"), &headerName, &headerValue);
}

/// <summary>
/// Writes an HTTP header to the ongoing response stream. Use only after beginResponse() has been called to initiate the HTTP
/// response.
/// </summary>
/// <param name="headerName">Name of the header, e.g. `Last-Modified`, as a PROGMEM string</param>
/// <param name="headerValue">The header value, e.g. `Sat, 14 Feb 2015 01:55:08 GMT`</param>
void HTTPServer::writeHeader_P(PGM_P headerName, const String& headerValue)
{
	writeHeader((__FlashStringHelper*)headerName, headerValue);
}

/// <summary>
/// Convenient function to write the Content-Type header. Use only after beginResponse() has been called to initiate the HTTP
/// response.
/// </summary>
/// <param name="contentType">String / regular C string representing the content type, e.g., `application/json`</param>
void HTTPServer::writeContentTypeHeader(const String& contentType)
{
	writeHeader_P(HTTP_HEADER_CONTENT_TYPE, contentType);
}

/// <summary>
/// Convenient function to write the Content-Type header. Use only after beginResponse() has been called to initiate the HTTP
/// response.
/// </summary>
/// <param name="contentType">PROGMEM string representing the content type, e.g., `application/json`.
/// See \ref HTTPConstants_ContentType for a list of pre-defined constants you can use</param>
void HTTPServer::writeContentTypeHeader_P(PGM_P contentType)
{
	String contentTypeStr((__FlashStringHelper*)contentType);
	writeContentTypeHeader(contentTypeStr);
}

/// <summary>
/// Indicates to the client that all headers have been written and that the response body follows.
/// </summary>
void HTTPServer::beginResponseBody()
{
	stage = HTTP_SERVER_STAGE_RESPONSE_BODY;
	write(F("\r\n"));
}

/// <summary>
/// Ends the connection and finalizes the response.
/// </summary>
void HTTPServer::endResponse()
{
	if (stage < HTTP_SERVER_STAGE_RESPONSE_BODY)
		beginResponseBody();

	push();
	close();

	stage = HTTP_SERVER_STAGE_RESPONSE_END;
}