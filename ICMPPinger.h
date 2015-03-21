// EtherDune ICMP Pinger class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements an ICMP Echo request service
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

/// \class ICMPPinger
/// \brief Implements an %ICMP Echo request service
/// \details This class allows you to send ping messages.
///
/// In order to process ping replies, create a derived class and override the onPingReply() virtual function
///
/// See PingTest.ino for an example

#ifndef _ICMPPINGER_H_
#define ICMPPINGER_H_

#include "ICMP.h"


class ICMPPinger : protected ICMP
{

	bool onICMPMessage();
	
public:

	void ping(const IPAddress& targetIP);

	virtual void onPingReply(uint16_t roundtripTime) = 0;
};



#endif