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
/// \details Implements the basics of DNS so as to resolve hostnames to IP addresses
/// See \ref DNSConfig for EtherFlow config parameters that apply to DNS.
///
/// In order to use DNS in your project, instantiate it in as a global in your .ino file this way:
/// \code
/// DNSClient net::DNS;
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
/// Etherflow is designed in such a way that if you do not instantiate the \ref DNSClient class 
/// in your code as above, it will not be compiled in, therefore saving code space if unused.
///
/// **Warning:** You will get linker errors if you do not instantiate DNS as indicated above and nevertheless attempt to
/// use methods that rely on DNS. My recommendation is that if you are unusure, always instantiate DNS 
/// and remove after your project is finished and see if it compiles.
///

#ifndef __DNS__
#define __DNS__

#include <ACross.h>
#include "UDPSocket.h"



class DNSClient : private UDPSocket
{

private:

	uint8_t timer;

	void onReceive(uint16_t len);
	bool sendPacket();
	void tick();

public:
	DNSClient();
	uint16_t resolve(const char* name);

};


#endif