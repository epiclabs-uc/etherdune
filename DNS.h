// EtherFlow DNS Service
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements the basics of DNS so as to resolve hostnames to IP addresses
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

/// \class DNSClient
/// \brief EtherFlow DNS Service
/// \details Implements the basics of DNS so as to resolve hostnames to IP addresses.
/// See \ref DNSConfig for EtherFlow config parameters that apply to DNS.
///
/// You cannot create an instance of this class. In order to use DNS in your project, 
/// access the singleton instance in net::DNS() this way:
/// \code
/// uint16_t id = net::DNS().resolve("www.friendev.com");
/// \endcode
///
/// To resolve a hostname to an IP Address, call the \ref resolve method and save the uint16_t result
/// as an identification token to match possible responses to your request.
///
/// To receive the reply, override the \ref onDNSResolve virtual function on any NetworkService-derived class
/// and match the identification to the token you received from \ref resolve.
///
/// Most classes have this mechanism already automatically implemented, providing overloads that take a host name
/// as opposed to an IP address.
///
/// Etherflow is designed in such a way that if you do not call `net::DNS()` within your code,
/// DNS functions won't be compiled in, thus saving space.
///
/// DNS code will be compiled in if, however, you use any other function within EtherFlow that in turn requires DNS to work
/// such as \ref HTTPClient::request, which takes in a hostname as a parameter and thus uses `net::DNS()` internally.
///
/// See DNSDemo.ino for a simple demonstration application that uses DNS alone.

#ifndef __DNS__
#define __DNS__

#include <ACross.h>
#include "UDPSocket.h"


class NetworkService;
class DNSClient : private UDPSocket
{
	friend class NetworkService;
private:

	uint8_t timer;

	void onReceive(uint16_t len);
	bool sendPacket();
	void tick();
	DNSClient();

public:
	
	uint16_t resolve(const char* name);

};


#endif