// EtherFlow HTTP Server class
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


/// \class HTTPServer
/// \brief Provides an easy way to build a web server
/// \details This class implements the basics of a web server engine
/// 
/// To use this class, create a derived class and override the appropriate virtual functions to receive
/// notifications about the progress of the incoming request and capture the needed data for your application
///
/// This class can only serve one request at a time. Combine with TCPListener to be able to "spawn" children
/// sockets that serve each request independently.
///
/// The flow of calls works as follows:
/// 1. onRequest() - called immediately after the first lime that contains the HTTP method and query string is received
/// 2. onHeaderReceived() - called once for each header in the request
/// 4. onBodyBegin() - called when all HTTP headers have been received and the body of the request is about to arrive
/// 5. onBodyReceived() - called once for each fragment of the body that is received.
/// 6. onRequestEnd() - called after all the body has been received
///
/// See HTTPServerDemo.ino for a full example on how to use this class.


#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <ACross.h>
#include "config.h"
#include "HTTPConstants.h"
#include "TCPSocket.h"
#include <FlowScanner.h>


class HTTPServer :public TCPSocket
{
private:

	static const uint8_t HTTP_SERVER_STAGE_INIT = 0;
	static const uint8_t HTTP_SERVER_STAGE_METHOD = 1;
	static const uint8_t HTTP_SERVER_STAGE_QUERY_STRING = 2;
	static const uint8_t HTTP_SERVER_STAGE_HEADERS = 3;
	static const uint8_t HTTP_SERVER_STAGE_BODY = 4;
	static const uint8_t HTTP_SERVER_STAGE_RESPONSE = 5;
	static const uint8_t HTTP_SERVER_STAGE_RESPONSE_BODY = 6;
	static const uint8_t HTTP_SERVER_STAGE_RESPONSE_END = 7;

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

	uint8_t stage;
	uint8_t crlfcount;
	FlowPattern requestPattern;
	FlowPattern headerPattern;
	FlowScanner scanner;

	void onReceive(uint16_t len, const byte* data);
	void onConnect();
	void onConnectRequest();
	void onClose();

protected:

	void resetParser();

public:

	uint8_t httpMethod;
	uint16_t contentLength;

	HTTPServer();

	void listen(uint16_t port = 80);
	void beginResponse(uint16_t statusCode, const String& message = "");
	void beginResponse_P(uint16_t statusCode, PGM_P message);
	void writeHeader(const String& headerName, const String& headerValue);
	void writeHeader(PGM_P headerName, const String& headerValue);
	void writeContentTypeHeader(const String& contentType);
	void writeContentTypeHeader_P(PGM_P contentType);
	void beginResponseBody();
	void endResponse();

	virtual void onBodyReceived(uint16_t len, const byte* data);
	virtual void onHeaderReceived(const char* headerName, const char* headerValue);
	virtual void onBodyBegin();
	virtual void onRequest(char* queryString);
	virtual void onRequestEnd();

};


#endif