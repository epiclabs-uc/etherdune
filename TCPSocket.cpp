// EtherFlow TCP implementation as a NetworkService
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

#include "TCPSocket.h"
#include "Checksum.h"

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("TCPSocket");

/// <summary>
/// Fires when the other party closed the incoming end of the connection because it has no more data
/// to send.
/// Note that the outgoing end of the connection must also be closed by calling close().
/// A socket is not fully closed until both directions are closed.
/// Usually you call close() as a response to the onClose() event, but this will depend
/// on the protocol being implemented.
/// </summary>
void TCPSocket::onClose() {}
/// <summary>
/// Fires when the socket connection is established. If this was a client socket, onConnect()
/// fires when SYN-ACK is received.
///
/// If this was a listening socket, onConnect fires when the ACK to the SYN-ACK is received,
/// that is, when the connection is fully established.
/// </summary>
void TCPSocket::onConnect() {}
/// <summary>
/// Called when a listening socket receives a connection request.
/// To accept the connection request, call accept()
/// </summary>
void TCPSocket::onConnectRequest() {}
/// <summary>
/// Called for each data packet received
/// </summary>
/// <param name="len">Length of the data received</param>
/// <param name="data">Pointer to data received</param>
void TCPSocket::onReceive(uint16_t len, const byte* data) {}
/// <summary>
/// Called when the socket is ready to be reused.
/// </summary>
void TCPSocket::onTerminate() {}

TCPSocket::TCPSocket()
{

	state = SCK_STATE_CLOSED;
	sequenceNumber = 0;
	stateTimer = 0;
	nextFlags.clear();


}

/// <summary>
/// Initiates a TCP connection to remoteIP and remotePort.
// Set these properties prior to calling connect()
/// </summary>
void TCPSocket::connect()
{
	
#if _DEBUG
	randomSeed((uint16_t)millis() + analogRead(A1) + analogRead(A5));
	localPort.l = random(255); 
#else
	localPort.l = srcPort_L_count++;
#endif
	localPort.h = TCP_SRC_PORT_H;
	ackNumber = 0;

	setState(SCK_STATE_SYN_SENT, SCK_TIMEOUT_SYN_SENT);
	sendSYN(false);
}

/// <summary>
/// Starts listening on the local port indicated by the
/// localPort property. Set this property prior to calling listen()
/// </summary>
void TCPSocket::listen()
{
	remoteIP.u = 0;
	setState(SCK_STATE_LISTEN, 0);
}

/// <summary>
/// Accepts a connection request that has been received on the
/// port this instance was listening on
/// </summary>
void TCPSocket::accept()
{
	remoteIP.u = packet.ip.sourceIP.u;
	remotePort = packet.tcp.sourcePort;

	ackNumber = packet.tcp.sequenceNumber + 1;

	sendSYN(true);
	setState(SCK_STATE_SYN_RECEIVED, SCK_TIMEOUT_SYN_RECEIVED);

}

/// <summary>
/// Accepts a connection request that was sent to another TCPSocket instance.
/// This allows to "spawn" a socket dedicated to fulfill a client request
/// while keeping the listening socket available to detect other connection
/// requests.
/// </summary>
/// <param name="listener">The TCPSocket that received the connection request</param>
void TCPSocket::accept(TCPSocket& listener)
{
	sequenceNumber = listener.sequenceNumber;
	localPort.rawValue = listener.localPort.rawValue;
	accept();

}

void TCPSocket::prepareTCPPacket(bool options, uint16_t dataLength)
{

	packet.ip.totalLength = dataLength + (options ?
		sizeof(IPHeader) + sizeof(TCPOptions) + sizeof(TCPHeader) :
		sizeof(IPHeader) + sizeof(TCPHeader));

	packet.ip.protocol = IP_PROTO_TCP;

	prepareIPPacket();

	packet.tcp.sourcePort = localPort;
	packet.tcp.destinationPort = remotePort;
	packet.tcp.sequenceNumber = sequenceNumber;
	packet.tcp.flags.clear();

	packet.tcp.windowSize = 512;
	packet.tcp.acknowledgementNumber = ackNumber;
	packet.tcp.checksum.zero();
	packet.tcp.urgentPointer.zero();

	packet.tcp.headerLength = options ? (sizeof(TCPHeader) + sizeof(TCPOptions)) / 4 : sizeof(TCPHeader) / 4;


}


void TCPSocket::sendSYN(bool ack)
{
	ACTRACE("sendSYN ACK=%d",ack);

	prepareTCPPacket(true, 0);

	packet.tcp.flags.SYN = 1;
	packet.tcp.flags.ACK = ack;

	packet.tcpOptions.option1 = 0x02;
	packet.tcpOptions.option1_length = 0x04;
	packet.tcpOptions.option1_value = TCP_MAXIMUM_SEGMENT_SIZE;


	packet.tcp.checksum.rawValue = calcTCPChecksum(true, 0, 0);

	sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader) + sizeof(TCPOptions));
}


/// <summary>
/// Immediately shuts down the socket and makes it available for a new task.
/// Call this method to ensure a full cleanup before reusing a socket instance.
/// </summary>
/// <remarks>
/// Note that using this method will not gracefully terminate a connection.
/// If a connection was established, no message will be sent to the other party.
/// </remarks>
void TCPSocket::terminate()
{
	setState(SCK_STATE_CLOSED, 0);
	ACASSERT(buffer.isEmpty(), "tx buffer not empty");
	buffer.flush();
	onTerminate();
}

/// <summary>
/// Attempts to gracefully close a connection.
/// </summary>
void TCPSocket::close()
{

	switch (state)
	{
		case SCK_STATE_CLOSE_WAIT:
		{
			setState(SCK_STATE_LAST_ACK, SCK_TIMEOUT_LAST_ACK);
		}break;
		case SCK_STATE_ESTABLISHED:
		{
			setState(SCK_STATE_FIN_WAIT_1, SCK_TIMEOUT_FIN_WAIT_1);
			//nextFlags.FIN = 1;
		}break;

	}
}


void TCPSocket::tick()
{


	ACDEBUG("tick/state=%S",getStateString());

	if (stateTimer == 1) //handle timeouts
	{
		ACTRACE("%S state timeout", getStateString());

		switch (state)
		{
			case SCK_STATE_SYN_RECEIVED:
			case SCK_STATE_SYN_SENT:
			case SCK_STATE_TIME_WAIT:
			case SCK_STATE_CLOSING:
			case SCK_STATE_FIN_WAIT_2:
			case SCK_STATE_FIN_WAIT_1:
			case SCK_STATE_LAST_ACK:
			{
				terminate();
			}return;


		}
	}

	if (stateTimer>0)
		stateTimer--;

	switch (state)
	{
		case SCK_STATE_SYN_SENT:
		{
			sendSYN(false);
		}break;

		case SCK_STATE_SYN_RECEIVED:
		{
			sendSYN(true);
		}break;

		case SCK_STATE_CLOSING:
		case SCK_STATE_FIN_WAIT_1:
		{
			if (!buffer.isEmpty())
				goto sck_state_established;
		}
		
		case SCK_STATE_LAST_ACK:
		{
			nextFlags.FIN = 1;
		}
		case SCK_STATE_FIN_WAIT_2:
		{
			nextFlags.ACK = 1;
		}
		case SCK_STATE_TIME_WAIT:
		case SCK_STATE_ESTABLISHED:
		{
			sck_state_established:
			processOutgoingBuffer();
		}

	}

	
}

bool TCPSocket::onPacketReceived()
{
	if (!(
		packet.eth.etherType == ETHTYPE_IP &&
		packet.ip.protocol == IP_PROTO_TCP &&
		state != SCK_STATE_CLOSED &&
		localPort.rawValue == packet.tcp.destinationPort.rawValue && 
		((remoteIP.u == packet.ip.sourceIP.u && remotePort.rawValue == packet.tcp.sourcePort.rawValue) || state == SCK_STATE_LISTEN)
		))
	{
		return false;
	}

	loadAll();

#if ENABLE_UDPTCP_RX_CHECKSUM

	if (!verifyUDPTCPChecksum())
	{
		ACWARN("TCP checksum error");
		return true;// drop packet, TCP checksum error
	}

#endif

	ACTRACE("Header SYN=%d ACK=%d FIN=%d RST=%d", packet.tcp.flags.SYN, packet.tcp.flags.ACK, packet.tcp.flags.FIN, packet.tcp.flags.RST)

	uint32_t incomingAckNum = packet.tcp.acknowledgementNumber;
	uint32_t incomingSeqNum = packet.tcp.sequenceNumber;

	int32_t bytesAck = (int32_t)(incomingAckNum - sequenceNumber);
	int32_t bytesReceived;

	ACTRACE("incomingAck=%lu localSeqNum=%lu bytesAck=%ld incomingSeqNum=%lu localAckNum=%lu",
		incomingAckNum, sequenceNumber, bytesAck, incomingSeqNum, ackNumber);

	if (packet.tcp.flags.RST)
	{
		if(state != SCK_STATE_LISTEN)
			terminate();

		return true;
	}

	if (bytesAck > 0)
		sequenceNumber += bytesAck;


	if (state == SCK_STATE_SYN_SENT && packet.tcp.flags.SYN && packet.tcp.flags.ACK)
	{
		ackNumber = incomingSeqNum + 1;
		setState(SCK_STATE_ESTABLISHED, 0);
		nextFlags.ACK = 1;
		onConnect();
		return true;
	}

	if (state == SCK_STATE_LISTEN && packet.tcp.flags.SYN)
	{
		onConnectRequest();
		return true;
	}

	if (state == SCK_STATE_SYN_RECEIVED)
	{
		if (!packet.tcp.flags.ACK)
			return true;

		setState(SCK_STATE_ESTABLISHED, 0);
		bytesAck--; // count one less byte (SYN counts as 1 "fake" byte)
		onConnect();
	}

	releaseWindow(bytesAck);
	
	if (ackNumber != incomingSeqNum)
	{
		ACDEBUG("dropped packet out of sequence.");
		nextFlags.ACK = 1;
		return true;
	}

	int16_t headerLength = sizeof(IPHeader) + packet.tcp.headerLength * 4;
	bytesReceived = packet.ip.totalLength - headerLength;
	ackNumber += bytesReceived;
	ACTRACE("bytesReceived=%ld", bytesReceived);

	

	if (bytesReceived > 0) // do not send an ACK if this was a packet with no data
	{
		nextFlags.ACK = 1;
		int16_t slen = min(bytesReceived, (int16_t)(sizeof(EthBuffer) - sizeof(EthernetHeader)) - headerLength);

		if (slen > 0)
			onReceive((uint16_t)slen, packet.raw + sizeof(EthernetHeader) + headerLength);
	}


	if (packet.tcp.flags.FIN)
	{
		ackNumber++;
	}


	switch (state)
	{
		case SCK_STATE_ESTABLISHED:
		{

			if (packet.tcp.flags.FIN)
			{
				setState(SCK_STATE_CLOSE_WAIT, 0);
				onClose();
			}


		}break;


		case SCK_STATE_FIN_WAIT_1:
		{
			if (bytesAck == 1 && buffer.isEmpty())
			{
				if (packet.tcp.flags.FIN)
				{
					setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);
					onClose();
				}
				else
					setState(SCK_STATE_FIN_WAIT_2, SCK_TIMEOUT_FIN_WAIT_2);
			}
			else
			{
				if (packet.tcp.flags.FIN)
				{
					setState(SCK_STATE_CLOSING, SCK_TIMEOUT_CLOSING);
					nextFlags.ACK = 1;
					onClose();
				}
			}

		}break;
		case SCK_STATE_CLOSING:
		{
			if (bytesAck == 1 && buffer.isEmpty())
				setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);
			else
			{
				nextFlags.ACK = 1;
			}

		}break;

		//case SCK_STATE_TIME_WAIT:
		case SCK_STATE_FIN_WAIT_2:
		{
			if (packet.tcp.flags.FIN)
			{
				setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);
				nextFlags.ACK = 1;
				onClose();
			}

		}break;

		case SCK_STATE_LAST_ACK:
		{
			terminate();
		}


		default:
			break;
	}



	return true;
}

void TCPSocket::processOutgoingBuffer()
{
	ACTRACE("processOutgoingBuffer");
	uint16_t dataLength = 0;
	uint16_t dataChecksum = 0;

	if (!buffer.isEmpty())
	{
		dataLength = buffer.fillTxBuffer(sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(TCPHeader), dataChecksum);
		nextFlags.ACK = 1;
	}

	if (nextFlags.raw !=0)
	{
		prepareTCPPacket(false, dataLength);
		packet.tcp.sequenceNumber = sequenceNumber;
		packet.tcp.flags = nextFlags;
		nextFlags.clear();
	
		ACTRACE("dataLength=%d dataChecksum=%d", dataLength, dataChecksum);

		packet.tcp.checksum.rawValue = calcTCPChecksum(false, dataLength, dataChecksum);

		sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader));

	}

}

/// <summary>
/// Sets the PSH TCP flag and also sends data in the outgoing buffer immediately.
/// Normally, socket writes have a maximum lag of 1 tick (by default 200ms) to try piggyback
/// ACKs and also attempt to send more data than just one write.
/// Using push() you can have EhterFlow send data without waiting the 200ms.
/// </summary>
void TCPSocket::push()
{
	nextFlags.PSH = true;
	processOutgoingBuffer();
}

void TCPSocket::releaseWindow(int32_t& bytesAck)
{

	while (bytesAck > 0 && !buffer.isEmpty())
		bytesAck -= buffer.release();

	ACASSERT(bytesAck >= 0, "released too much. bytesAck=%d", bytesAck);

}

__FlashStringHelper* TCPSocket::getStateString()
{
	const char* s;

	switch (state)
	{
		case SCK_STATE_CLOSED: s = PSTR("CLOSED"); break;
		case SCK_STATE_LISTEN: s = PSTR("LISTEN"); break;
		case SCK_STATE_SYN_SENT: s = PSTR("SYN_SENT"); break;
		case SCK_STATE_SYN_RECEIVED: s = PSTR("SYN_RECEIVED"); break;
		case SCK_STATE_ESTABLISHED: s = PSTR("ESTABLISHED"); break;
		case SCK_STATE_FIN_WAIT_1: s = PSTR("FIN_WAIT_1"); break;
		case SCK_STATE_FIN_WAIT_2: s = PSTR("FIN_WAIT_2"); break;
		case SCK_STATE_CLOSE_WAIT: s = PSTR("CLOSE_WAIT"); break;
		case SCK_STATE_CLOSING: s = PSTR("CLOSING"); break;
		case SCK_STATE_LAST_ACK: s = PSTR("LAST_ACK"); break;
		case SCK_STATE_TIME_WAIT: s = PSTR("TIME_WAIT"); break;
		case SCK_STATE_RESOLVING: s = PSTR("RESOLVING NAME"); break;

		default:
			s = PSTR("UNKNOWN");
	}

	return (__FlashStringHelper*)s;

}