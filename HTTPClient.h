// EtherFlow HTTP Client class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Provides an easy way to query a web server
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

/// \class HTTPClient
/// \brief Provides an easy way to query a web server
/// \details This class extends TCPSocket by adding methods that are helpful when connecting to a web server.
/// 
/// To use this class, create a derived class and override the appropriate virtual functions to receive
/// notifications about the progress of the request and capture the needed data for your application
///
/// The flow of calls works as follows:
/// 1. call request()
/// 2. onResponseReceived() - called immediately after the first line is received and there is a status code, e.g. `200 OK`
/// 3. onHeaderReceived() - called once for each fragment of the header portion of the response
/// 4. onBodyBegin() - called when all HTTP headers have been received and the body of the response is about to arrive
/// 5. onBodyReceived() - called once for each fragment of the body that is received.
/// 6. onResponseEnd() - called after all the body has been received
///
/// See HTTPClientDemo_REST.ino for a full example on how to use this class.

#include <ACross.h>
#include "TCPSocket.h"
#include <FlowScanner.h>

static const char statusCodePatternString[] PROGMEM = "HTTP/%*1d.%*1d %d%*99[^\r\n]\r\n";
static const char bodyBeginPatternString[] PROGMEM = "\r\n\r\n";

class HTTPClient : public TCPSocket
{

private:

	FlowPattern statusCodePattern;
	FlowPattern bodyBeginPattern;
	String host;
	String res;
	uint16_t DNSid;

	void onConnect();
	void onReceive(uint16_t len, const byte* data);
	void onClose();
	void onDNSResolve(uint8_t status, uint16_t identification, const IPAddress& ip);
	
protected:

	FlowScanner scanner;//!< Internal FlowScanner instance used to detect header/response/status code, etc. 
						//!< Can be reused to scan the body after onBodyBegin() is called by EtherFlow

public:

	uint16_t statusCode; //!< Contains the HTTP status code of the response. Valid only after onResponseReceived() is called by EtherFlow

	HTTPClient();
	~HTTPClient();

	void request(const String& hostName, const String& resource, uint16_t port=80);

	virtual void onResponseReceived();
	virtual void onHeaderReceived(uint16_t len, const byte* data);
	virtual void onBodyReceived(uint16_t len, const byte* data);
	virtual void onBodyBegin();
	virtual void onResponseEnd();

};

