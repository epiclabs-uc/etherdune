#ifndef _TCPLISTENER_H_
#define _TCPLISTENER_H_

#include "TCPSocket.h"


//SOCKET is expected to derive from TCPSocket
template <class SOCKET, uint8_t MAX_CLIENTS>
class TCPListener : public TCPSocket
{
protected:
	SOCKET clients[MAX_CLIENTS];
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
	void listen(uint16_t port)
	{
		localPort.setValue(port);
		TCPSocket::listen();
	}



};


#endif