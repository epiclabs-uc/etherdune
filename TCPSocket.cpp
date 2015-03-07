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
	randomSeed((uint16_t)millis() + analogRead(A1) + analogRead(A5));
	localPort.l = random(255); //srcPort_L_count++;
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
	remoteIP.u = chunk.ip.sourceIP.u;
	remotePort = chunk.tcp.sourcePort;

	ackNumber = chunk.tcp.sequenceNumber.getValue() + 1;

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

	chunk.ip.totalLength.setValue(dataLength + (options ?
		sizeof(IPHeader) + sizeof(TCPOptions) + sizeof(TCPHeader) :
		sizeof(IPHeader) + sizeof(TCPHeader)));

	chunk.ip.protocol = IP_PROTO_TCP_V;

	prepareIPPacket();

	chunk.tcp.sourcePort = localPort;
	chunk.tcp.destinationPort = remotePort;
	chunk.tcp.sequenceNumber.setValue(sequenceNumber);
	chunk.tcp.flags.clear();

	chunk.tcp.windowSize.setValue(512);
	chunk.tcp.acknowledgementNumber.setValue(ackNumber);
	chunk.tcp.checksum.zero();
	chunk.tcp.urgentPointer.zero();

	chunk.tcp.headerLength = options ? (sizeof(TCPHeader) + sizeof(TCPOptions)) / 4 : sizeof(TCPHeader) / 4;


}


void TCPSocket::sendSYN(bool ack)
{
	ACTRACE("sendSYN ACK=%d",ack);

	prepareTCPPacket(true, 0);

	chunk.tcp.flags.SYN = 1;
	chunk.tcp.flags.ACK = ack;

	chunk.tcpOptions.option1 = 0x02;
	chunk.tcpOptions.option1_length = 0x04;
	chunk.tcpOptions.option1_value.setValue(TCP_MAXIMUM_SEGMENT_SIZE);


	chunk.tcp.checksum.rawu = calcTCPChecksum(true, 0, 0);

	sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader) + sizeof(TCPOptions));
}


void TCPSocket::terminate()
{
	setState(SCK_STATE_CLOSED, 0);
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
		case SCK_STATE_LAST_ACK:
		{
			/*sequenceNumber--;*/
			nextFlags.FIN = 1;

		}
		case SCK_STATE_FIN_WAIT_2:
		case SCK_STATE_TIME_WAIT:
		{
			nextFlags.ACK = 1;
		}
		case SCK_STATE_ESTABLISHED:
		{
			processOutgoingBuffer();
		}

	}

	
}

bool TCPSocket::onPacketReceived()
{
	if (!(
		chunk.eth.etherType.getValue() == ETHTYPE_IP &&
		chunk.ip.protocol == IP_PROTO_TCP_V &&
		state != SCK_STATE_CLOSED &&
		localPort.rawu == chunk.tcp.destinationPort.rawu && 
		((remoteIP.u == chunk.ip.sourceIP.u && remotePort.rawu == chunk.tcp.sourcePort.rawu) || state == SCK_STATE_LISTEN)
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

	ACTRACE("Header SYN=%d ACK=%d FIN=%d RST=%d", chunk.tcp.flags.SYN, chunk.tcp.flags.ACK, chunk.tcp.flags.FIN, chunk.tcp.flags.RST)

	uint32_t incomingAckNum = chunk.tcp.acknowledgementNumber.getValue();
	uint32_t incomingSeqNum = chunk.tcp.sequenceNumber.getValue();

	int32_t bytesAck = (int32_t)(incomingAckNum - sequenceNumber);
	int32_t bytesReceived;

	ACTRACE("incomingAck=%lu localSeqNum=%lu bytesAck=%ld incomingSeqNum=%lu localAckNum=%lu",
		incomingAckNum, sequenceNumber, bytesAck, incomingSeqNum, ackNumber);

	if (chunk.tcp.flags.RST && state != SCK_STATE_LISTEN)
	{
		terminate();
		return false;
	}

	if (bytesAck > 0)
		sequenceNumber += bytesAck;


	if (state == SCK_STATE_SYN_SENT && chunk.tcp.flags.SYN && chunk.tcp.flags.ACK)
	{
		ackNumber = incomingSeqNum + 1;
		setState(SCK_STATE_ESTABLISHED, 0);
		onConnect();
		nextFlags.ACK = 1;
		return false;
	}

	if (state == SCK_STATE_LISTEN && chunk.tcp.flags.SYN)
	{
		onConnect();
		return false;
	}

	if (state == SCK_STATE_SYN_RECEIVED)
	{
		if (!chunk.tcp.flags.ACK)
			return false;

		setState(SCK_STATE_ESTABLISHED, 0);
		bytesAck--; // count one less byte (SYN counts as 1 "fake" byte)
	}

	if (ackNumber != incomingSeqNum)
	{
		ACDEBUG("dropped packet out of sequence.");
		nextFlags.ACK = 1;
		return false;
	}

	int16_t headerLength = sizeof(IPHeader) + chunk.tcp.headerLength * 4;
	bytesReceived = chunk.ip.totalLength.getValue() - headerLength;
	ackNumber += bytesReceived;
	ACTRACE("bytesReceived=%ld", bytesReceived);

	releaseWindow(bytesAck);

	if (bytesReceived > 0) // do not send an ACK if this was a packet with no data
		nextFlags.ACK = 1;


	if (chunk.tcp.flags.FIN)
	{
		ackNumber++;
	}


	switch (state)
	{
		case SCK_STATE_ESTABLISHED:
		{
			int16_t slen = min(bytesReceived, (int16_t)(sizeof(EthBuffer) - sizeof(EthernetHeader)) - headerLength);

			if (slen > 0)
				onReceive((uint16_t)slen, chunk.raw + sizeof(EthernetHeader) + headerLength);


			if (chunk.tcp.flags.FIN)
			{
				setState(SCK_STATE_CLOSE_WAIT, 0);
				onClose();
			}


		}break;


		case SCK_STATE_FIN_WAIT_1:
		{
			if (bytesAck == 1 && buffer.isEmpty())
			{
				if (chunk.tcp.flags.FIN)
					setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);
				else
					setState(SCK_STATE_FIN_WAIT_2, SCK_TIMEOUT_FIN_WAIT_2);
			}

		}break;
		case SCK_STATE_FIN_WAIT_2:
		{
			if (chunk.tcp.flags.FIN)
				setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);

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
		dataLength = buffer.fillTxBuffer(sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(TCPHeader), dataChecksum);

	if (nextFlags.raw !=0 || dataLength !=0)
	{
		prepareTCPPacket(false, dataLength);
		chunk.tcp.sequenceNumber.setValue(sequenceNumber);
		chunk.tcp.flags = nextFlags;
		nextFlags.clear();
	
		ACTRACE("dataLength=%d dataChecksum=%d", dataLength, dataChecksum);

		chunk.tcp.checksum.rawu = calcTCPChecksum(false, dataLength, dataChecksum);

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