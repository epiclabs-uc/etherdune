// EtherDune TCP implementation as a NetworkService
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements the TCP protocol
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
\class TCPSocket
\brief Implements the TCP protocol
\details This class implements the TCP protocol as a NetworkService.

To consume this class, create a derived class of TCPSocket to be able to override
the different virtual functions that notify of events related to the socket.

The socket write functions are inherited from Socket. Check out Socket::write()
for more information.

See TCPClientDemo_DaytimeClient.ino, ChatServer.ino and EchoServer.ino for
examples

*/



#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

#include <ACross.h>

#include "Socket.h"
#include "Stateful.h"

class TCPSocket;


class TCPSocket : public Socket, public Stateful
{

private:

	uint32_t sequenceNumber;
	uint32_t ackNumber;
	TCPFlags nextFlags;

	bool onPacketReceived();
	void processOutgoingBuffer();
	void prepareTCPPacket(bool options, uint16_t dataLength);
	void releaseWindow(int32_t& bytesAck);
	void tick();
	void sendSYN(bool ack);
	__FlashStringHelper* getStateString();

public:

	TCPSocket();

	void connect();
	void close();
	void terminate();
	void listen();
	void accept();
	void accept(TCPSocket& listener);
	void push();

	virtual void onConnect();
	virtual void onConnectRequest();
	virtual void onClose();
	virtual void onReceive(uint16_t len, const byte* data);
	virtual void onTerminate();

};



#endif
