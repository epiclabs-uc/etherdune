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


/// \class HTTPServer
/// \brief Provides an easy way to build a web server
/// \details This class implements the basics of a web server engine
/// 
/// To use this class, create a derived class and override the appropriate virtual functions to receive
/// notifications about the progress of the incoming request and capture the needed data for your application
///
/// This class can only serve one request at a time. Combine with TCPListener to be able to "spawn" children
/// sockets that serve each request independently. (see HTTPServerDemo_MultipleClients.ino example)
///
/// The flow of calls works as follows:
/// 1. onRequest() - called immediately after the first line that contains the HTTP method and query string is received
/// 2. onHeaderReceived() - called once for each header in the request
/// 4. onBodyBegin() - called when all HTTP headers have been received and the body of the request is about to arrive
/// 5. onBodyReceived() - called once for each fragment of the body that is received.
/// 6. onRequestEnd() - called after all the body has been received
///
/// Depending on what information your server is interested in in order to be able to service the request, you
/// may initiate your response at any of the above event handlers. For example if you are only interested in the query string
/// you can override onRequest() only and build your response there on the spot.
///
/// To build a response, call the following functions:
/// 1.- beginResponse() or beginResponse_P(), to send a status code and optional message
/// 2.- Any number of calls to writeHeader() overload, if any, to send the different HTTP response headers.
/// 3.- beginResponseBody() to mark the beginning of the response body.
/// 4.- Finally, call endResponse() to indicate the other party the response is over.
///
/// See HTTPServerDemo.ino and HTTPServerDemo_MultipleClients.ino for examples on how to use this class.


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

	/// \cond
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
	/// \endcond

	uint8_t stage;
	uint8_t crlfcount;
	FlowPattern requestPattern;
	FlowPattern headerPattern;
	FlowScanner scanner;

	void onReceive(uint16_t len, const byte* data);
	void onConnect();
	void onConnectRequest();
	void onClose();
	void resetParser();

protected:

	

public:

	uint8_t httpMethod; //!< Indicates what HTTP method was used in this request. Valid only after onRequest() has been called.
						//!< see \ref HTTPConstants_Methods for a list of possible values.
	
	uint16_t contentLength; //!< captured length of the incoming request, in bytes. Valid only after onBodyBegin() is called.
							//!< `contentLength` will be zero if no `Content-Length` header was present in the request.

	HTTPServer();

	void listen(uint16_t port = 80);
	void beginResponse(uint16_t statusCode, const String& message = "");
	void beginResponse_P(uint16_t statusCode, PGM_P message);
	void writeHeader(const String& headerName, const String& headerValue);
	void writeHeader_P(PGM_P headerName, const String& headerValue);
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