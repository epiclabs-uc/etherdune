// EtherDune UDP implementation as a NetworkService
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements the UDP protocol
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

/**
\class UDPSocket
\brief Implements the UDP protocol
\details This class implements the UDP protocol as a NetworkService.

To consume this class, create a derived class of UDPSocket to be able to override
the different virtual functions that notify of events related to the socket.

The socket write functions are inherited from Socket. Check out Socket::write()
for more information.

*/
#ifndef __UDPSOCKET_H_
#define __UDPSOCKET_H_

#include "Socket.h"


class UDPSocket : public Socket
{

private:

	bool onPacketReceived();
	bool sending;

protected:

	void prepareUDPPacket(uint16_t dataLength, uint16_t dataChecksum);

	virtual bool sendPacket();
	void tick();

public:

	UDPSocket();

	bool send();
	

	virtual void onReceive(uint16_t len);

};







#endif