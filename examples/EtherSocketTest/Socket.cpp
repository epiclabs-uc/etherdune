// 
// 
// 

#include "Socket.h"

Socket Socket::sockets[MAX_TCP_SOCKETS];

Socket::Socket()
{

}

void Socket::begin()
{
	activeSockets = 0;
	for (int i = 0; i < MAX_TCP_SOCKETS; i++)
	{
		Socket* s = &sockets[i];
		s->id = i;
		s->srcPort_L = 0;
	}
}


Socket* Socket::create(SocketCallback eventHandlerCallback)
{
	uint16_t mask = 1;
	for (int i = 0; i < MAX_TCP_SOCKETS; i++)
	{
		if ((mask & activeSockets) == 0)
		{
			activeSockets |= mask;
			Socket* s = &sockets[i];
			s->eventHandler = eventHandlerCallback;
			return s;
		}
	}

	return NULL;
}

void Socket::release()
{
	activeSockets &= ~(1 << id);
}

void Socket::connect(IPAddress& ip, uint16_t port)
{
	dstAddr = ip;
	dstPort.setValue(port);
	srcPort_L++;

	EtherSocket::chunk.tcp.sourcePort.h = TCP_SRC_PORT_H;
	EtherSocket::chunk.tcp.sourcePort.l = srcPort_L;
	EtherSocket::chunk.tcp.destinationPort = dstPort;
}