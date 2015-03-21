// EtherDune ICMP automatic Echo reply class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements an ICMP Echo reply service
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

/// \class ICMPPingAutoReply
/// \brief Implements an ICMP Echo reply service
/// \details By instantiating this class and keeping the object alive, your
/// application will respond to all incoming ping requests.
/// Useful to see from the network if your microcontroller is alive!
///
/// This class has no public members. To use it, simply put the following code in your .ino file, outside of any function:
///
/// \code
/// #include <ICMPAutoReply.h>
/// 
/// ICMPAutoReply echo;
/// \endcode
///
/// See PingTest.ino for an example


#ifndef _ICMPAUTOREPLY_H_
#define _ICMPAUTOREPLY_H_

#include <ACross.h>
#include "ICMP.h"

class ICMPPingAutoReply : protected ICMP
{

private:

	bool onICMPMessage();

};


#endif
