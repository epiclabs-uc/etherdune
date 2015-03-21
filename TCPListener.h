// EtherDune TCP Listener helper template class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Maintains a list of available sockets, also listening and spawning sockets to
// serve multiple clients simultaneously.
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
\class TCPListener
\brief  Maintains a list of available sockets, also listening and spawning sockets to
serve multiple clients simultaneously.
\details This class starts a listening socket that accepts all connection requests 
and tries to assign the workload of serving each request to an available socket.

The template takes two parameters:

SOCKET: a type that is expected to derive from TCPSocket
MAX_CLIENTS: maximum number of simultaneous connections to accomodate.
	TCPListener will statically allocate an array of MAX_CLIENT instances of SOCKET, so size MAX_CLIENTS carefully.

Each time a new connection is established, onConnect() will be called
on the corresponding SOCKET instance.

See HTTPServerDemo_MultipleClients.ino for a working example.
Compare with HTTPServerDemo.ino to see the difference

*/
#ifndef _TCPLISTENER_H_
#define _TCPLISTENER_H_

#include "TCPSocket.h"


//SOCKET is expected to derive from TCPSocket
template <class SOCKET, uint8_t MAX_CLIENTS>
class TCPListener : public TCPSocket
{
protected:
	SOCKET clients[MAX_CLIENTS];
	/// <summary>
	/// Called when there are no more sockets available
	/// </summary>
	virtual void onNoMoreConnections(){}

protected:
	
	void onTerminate()
	{
		TCPSocket::listen();
	}
	void onConnectRequest()
	{

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			SOCKET& sck = clients[i];
			if (sck.getState() == SCK_STATE_CLOSED)
			{
				sck.accept(*this);

				return;
			}
		}

		onNoMoreConnections();

	}

public:
	/// <summary>
	/// Starts to listen on the specified TCP port.
	/// </summary>
	/// <param name="port">The port to listen on</param>
	void listen(uint16_t port)
	{
		localPort = port;
		TCPSocket::listen();
	}



};


#endif