// 
// 
// 

#include "TCPSocket.h"
#include "Checksum.h"

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("TCPSocket");

void TCPSocket::onClose() {}
void TCPSocket::onConnect() {}
void TCPSocket::onConnectRequest() {}
void TCPSocket::onReceive(uint16_t len, const byte* data) {}
void TCPSocket::onTerminate() {}

TCPSocket::TCPSocket()
{

	state = SCK_STATE_CLOSED;
	sequenceNumber = 0;
	stateTimer = 0;
	nextFlags.clear();


}

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

void TCPSocket::listen()
{
	remoteIP.u = 0;
	setState(SCK_STATE_LISTEN, 0);
}

void TCPSocket::accept()
{
	remoteIP.u = packet.ip.sourceIP.u;
	remotePort = packet.tcp.sourcePort;

	ackNumber = packet.tcp.sequenceNumber.getValue() + 1;

	sendSYN(true);
	setState(SCK_STATE_SYN_RECEIVED, SCK_TIMEOUT_SYN_RECEIVED);

}

void TCPSocket::accept(TCPSocket& listener)
{
	sequenceNumber = listener.sequenceNumber;
	localPort.rawu = listener.localPort.rawu;
	accept();

}

void TCPSocket::prepareTCPPacket(bool options, uint16_t dataLength)
{

	packet.ip.totalLength.setValue(dataLength + (options ?
		sizeof(IPHeader) + sizeof(TCPOptions) + sizeof(TCPHeader) :
		sizeof(IPHeader) + sizeof(TCPHeader)));

	packet.ip.protocol = IP_PROTO_TCP;

	prepareIPPacket();

	packet.tcp.sourcePort = localPort;
	packet.tcp.destinationPort = remotePort;
	packet.tcp.sequenceNumber.setValue(sequenceNumber);
	packet.tcp.flags.clear();

	packet.tcp.windowSize.setValue(512);
	packet.tcp.acknowledgementNumber.setValue(ackNumber);
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
	packet.tcpOptions.option1_value.setValue(TCP_MAXIMUM_SEGMENT_SIZE);


	packet.tcp.checksum.rawu = calcTCPChecksum(true, 0, 0);

	sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader) + sizeof(TCPOptions));
}


void TCPSocket::terminate()
{
	setState(SCK_STATE_CLOSED, 0);
	ACASSERT(buffer.isEmpty(), "tx buffer not empty");
	buffer.flush();
	onTerminate();
}

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
			nextFlags.FIN = 1;
		}break;

		default:
		{
			terminate(); return;
		}
		break;
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

		case SCK_STATE_FIN_WAIT_1:
		{
			if (!buffer.isEmpty())
				goto sck_state_established;
		}
		case SCK_STATE_CLOSING:
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
		packet.eth.etherType.getValue() == ETHTYPE_IP &&
		packet.ip.protocol == IP_PROTO_TCP &&
		state != SCK_STATE_CLOSED &&
		localPort.rawu == packet.tcp.destinationPort.rawu && 
		((remoteIP.u == packet.ip.sourceIP.u && remotePort.rawu == packet.tcp.sourcePort.rawu) || state == SCK_STATE_LISTEN)
		))
	{
		return false;
	}

	loadAll();

#if ENABLE_UDPTCP_RX_CHECKSUM

	if (!verifyTCPChecksum())
	{
		ACWARN("TCP checksum error");
		return true;// drop packet, TCP checksum error
	}

#endif

	ACTRACE("Header SYN=%d ACK=%d FIN=%d RST=%d", packet.tcp.flags.SYN, packet.tcp.flags.ACK, packet.tcp.flags.FIN, packet.tcp.flags.RST)

	uint32_t incomingAckNum = packet.tcp.acknowledgementNumber.getValue();
	uint32_t incomingSeqNum = packet.tcp.sequenceNumber.getValue();

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
	bytesReceived = packet.ip.totalLength.getValue() - headerLength;
	ackNumber += bytesReceived;
	ACTRACE("bytesReceived=%ld", bytesReceived);

	

	if (bytesReceived > 0) // do not send an ACK if this was a packet with no data
		nextFlags.ACK = 1;


	if (packet.tcp.flags.FIN)
	{
		ackNumber++;
	}


	switch (state)
	{
		case SCK_STATE_ESTABLISHED:
		{
			int16_t slen = min(bytesReceived, (int16_t)(sizeof(EthBuffer) - sizeof(EthernetHeader)) - headerLength);

			if (slen > 0)
				onReceive((uint16_t)slen, packet.raw + sizeof(EthernetHeader) + headerLength);


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
					setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);
				else
					setState(SCK_STATE_FIN_WAIT_2, SCK_TIMEOUT_FIN_WAIT_2);
			}
			else
			{
				if (packet.tcp.flags.FIN)
				{
					setState(SCK_STATE_CLOSING, SCK_TIMEOUT_CLOSING);
					nextFlags.ACK = 1;
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

		case SCK_STATE_TIME_WAIT:
		case SCK_STATE_FIN_WAIT_2:
		{
			if (packet.tcp.flags.FIN)
			{
				setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);
				nextFlags.ACK = 1;
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
		packet.tcp.sequenceNumber.setValue(sequenceNumber);
		packet.tcp.flags = nextFlags;
		nextFlags.clear();
	
		ACTRACE("dataLength=%d dataChecksum=%d", dataLength, dataChecksum);

		packet.tcp.checksum.rawu = calcTCPChecksum(false, dataLength, dataChecksum);

		sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader));

	}

}

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