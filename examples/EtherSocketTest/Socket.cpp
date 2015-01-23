// 
// 
// 

#include "Socket.h"

uint8_t Socket::srcPort_L_count = 0;


void Socket::onClose() {}
void Socket::onConnect() {}
void Socket::onReceive(uint16_t len, const byte* data) {}

Socket::Socket()
{
	
	state = SCK_STATE_CLOSED;
	sequenceNumber = 0;
	EtherSocket::registerSocket(*this);
}

Socket::~Socket()
{
	EtherSocket::unregisterSocket(*this);
}


void Socket::connect()
{
	srandom(millis() + analogRead(A1) + analogRead(A5));
	srcPort_L = random();//srcPort_L_count++;
	ackNumber = 0;

	setState(SCK_STATE_SYN_SENT,SCK_TIMEOUT_SYN_SENT);
	sendSYN();



}

void Socket::preparePacket(bool options, uint16_t dataLength)
{
	EtherSocket::chunk.ip.version = 4;
	EtherSocket::chunk.ip.IHL = 0x05; //20 bytes
	EtherSocket::chunk.ip.raw[1] = 0x00; //DSCP/ECN=0;
	
	EtherSocket::chunk.ip.totalLength.setValue(dataLength + (options ? 
		sizeof(IPHeader) + sizeof(TCPOptions) + sizeof(TCPHeader) :
		sizeof(IPHeader) + sizeof(TCPHeader)));

	EtherSocket::chunk.ip.identification.setValue(0);
	EtherSocket::chunk.ip.flags = 0;
	EtherSocket::chunk.ip.fragmentOffset = 0;
	EtherSocket::chunk.ip.protocol = IP_PROTO_TCP_V;
	EtherSocket::chunk.ip.checksum.setValue(0);
	EtherSocket::chunk.ip.sourceIP = EtherSocket::localIP;
	EtherSocket::chunk.ip.destinationIP = remoteAddress;
	EtherSocket::chunk.ip.TTL = 255;
	EtherSocket::chunk.ip.checksum.setValue(~EtherSocket::checksum(0, (uint8_t*)&EtherSocket::chunk.ip, sizeof(IPHeader)));

	EtherSocket::chunk.tcp.sourcePort.h = TCP_SRC_PORT_H;
	EtherSocket::chunk.tcp.sourcePort.l = srcPort_L;
	EtherSocket::chunk.tcp.destinationPort = remotePort;
	EtherSocket::chunk.tcp.sequenceNumber.setValue(sequenceNumber);
	EtherSocket::chunk.tcp.flags = 0x00;

	EtherSocket::chunk.tcp.windowSize.setValue(512);
	EtherSocket::chunk.tcp.acknowledgementNumber.setValue(ackNumber);
	EtherSocket::chunk.tcp.checksum.zero();
	EtherSocket::chunk.tcp.urgentPointer.zero();

	EtherSocket::chunk.tcp.headerLength = options ? (sizeof(TCPHeader) + sizeof(TCPOptions)) / 4 : sizeof(TCPHeader) / 4;

	
}

void Socket::calcTCPChecksum(bool options, uint16_t dataLength, uint16_t dataChecksum)
{
	uint16_t sum;
	uint8_t headerLength = options ? sizeof(TCPOptions) + sizeof(TCPHeader) : sizeof(TCPHeader);
	
	nint32_t pseudo;
	pseudo.h.h = 0;
	pseudo.h.l = IP_PROTO_TCP_V;
	pseudo.l.setValue(dataLength + headerLength);

	sum = EtherSocket::checksum(0, (uint8_t*)&EtherSocket::chunk.ip.sourceIP, sizeof(IPAddress) * 2);
	sum = EtherSocket::checksum(sum, (uint8_t*)&pseudo, sizeof(pseudo));
	sum = EtherSocket::checksum(sum, (uint8_t*)&EtherSocket::chunk.tcp, headerLength);

	sum += dataChecksum;

	if (sum < dataChecksum)
	{
		dprintln("carry??");
		sum++;
	}

	EtherSocket::chunk.tcp.checksum.setValue(~sum);
}

void Socket::setState(uint8_t newState, uint8_t timeout)
{
	state = newState;
	stateTimer = timeout;

	dprint("setState="); DEBUG(printState());
}

void Socket::sendSYN()
{
	dprintln("sendSYN()");

	preparePacket(true,0);
	
	EtherSocket::chunk.tcp.SYN = 1;
	
	EtherSocket::chunk.tcpOptions.option1 = 0x02;
	EtherSocket::chunk.tcpOptions.option1_length = 0x04;
	EtherSocket::chunk.tcpOptions.option1_value.setValue(TCP_MAXIMUM_SEGMENT_SIZE);
	

	calcTCPChecksum(true, 0,0);

	EtherSocket::sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader) + sizeof(TCPOptions));
}

void Socket::sendFIN()
{
	dprintln("sendFIN()");

	preparePacket(false, 0);

	EtherSocket::chunk.tcp.FIN = 1;
	EtherSocket::chunk.tcp.ACK = 1;

	calcTCPChecksum(false, 0, 0);


	EtherSocket::sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader));
}


void Socket::terminate()
{
	setState(SCK_STATE_CLOSED, 0);
}

void Socket::close()
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


	sendFIN();
	sequenceNumber++;


}


void Socket::tick()
{
	
	dprint("state="); DEBUG(printState());
	
	if (stateTimer==1) //handle timeouts
	{
		switch (state)
		{
			case SCK_STATE_SYN_SENT:
			{
				onClose();
			}//fall back to terminate() below
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
			sendSYN();
		}break;

		case SCK_STATE_ESTABLISHED:
		{
			processOutgoingBuffer();
		}break;

		case SCK_STATE_FIN_WAIT_1:
		case SCK_STATE_LAST_ACK:
		{
			sequenceNumber--;
			sendFIN();

		}break;
	}
}


bool Socket::processSegment(bool isHeader, uint16_t len)
{
	dprint("\n___________\nsegment received:");


	if (isHeader)
	{
		dprintln("Header");
		dprint("SYN="); dprint(EtherSocket::chunk.tcp.SYN);	
		dprint(";ACK="); dprint(EtherSocket::chunk.tcp.ACK);
		dprint(";FIN="); dprint(EtherSocket::chunk.tcp.FIN);
		dprint(";RST="); dprintln(EtherSocket::chunk.tcp.RST);

		uint32_t incomingAckNum = EtherSocket::chunk.tcp.acknowledgementNumber.getValue();
		uint32_t incomingSeqNum = EtherSocket::chunk.tcp.sequenceNumber.getValue();

		int32_t bytesAck = (int32_t)(incomingAckNum - sequenceNumber);
		int32_t bytesReceived; // = (int32_t)(incomingSeqNum - ackNumber);

		dprint("incomingAck="); dprintln(incomingAckNum);
		dprint("localSeqNum="); dprintln(sequenceNumber);
		dprint("bytesAck="); dprintln(bytesAck);

		dprint("incomingSeqNum="); dprintln(incomingSeqNum);
		dprint("localAckNum="); dprintln(ackNumber);

		if (EtherSocket::chunk.tcp.RST)
		{
			terminate();
			return false;
		}

		if (bytesAck >0)
			sequenceNumber += bytesAck;


		if (state == SCK_STATE_SYN_SENT && EtherSocket::chunk.tcp.SYN && EtherSocket::chunk.tcp.ACK)
		{
			ackNumber = incomingSeqNum + 1;
			setState(SCK_STATE_ESTABLISHED,0);
			writeBuffer(0, 0);
			onConnect();
			return false;
		}

		if (ackNumber != incomingSeqNum)
		{
			dprintln("dropped packet out of sequence.");
			return false;
		}

		uint16_t headerLength = sizeof(IPHeader) + EtherSocket::chunk.tcp.headerLength * 4;
		bytesReceived = EtherSocket::chunk.ip.totalLength.getValue() - headerLength;
		ackNumber += bytesReceived;
		dprint("bytesReceived="); dprintln(bytesReceived);

		releaseWindow(bytesAck);
		writeBuffer(0, 0); // trigger send an ACK of what we have.

		if (EtherSocket::chunk.tcp.FIN)
			ackNumber++;


		switch (state)
		{
			case SCK_STATE_ESTABLISHED:
			{
				uint16_t slen = min(bytesReceived, sizeof(EthBuffer) - sizeof(EthernetHeader) - headerLength);
				
				if (slen>0)
					onReceive(slen, EtherSocket::chunk.raw + sizeof(EthernetHeader) + headerLength);

				if (EtherSocket::chunk.tcp.FIN)
				{
					setState(SCK_STATE_CLOSE_WAIT, 0);
					//sendFIN();
					onClose();
				}


			}break;


			case SCK_STATE_FIN_WAIT_1:
			{
				if (EtherSocket::chunk.tcp.FIN)
					setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);
				else
					setState(SCK_STATE_FIN_WAIT_2, SCK_TIMEOUT_FIN_WAIT_2);

			}break;
			case SCK_STATE_FIN_WAIT_2:
			{
				if (EtherSocket::chunk.tcp.FIN)
					setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);

			}break;

			case SCK_STATE_LAST_ACK:
			{
				setState(SCK_STATE_CLOSED,0);
			}


			default:
				break;
		}

		
		
		
		


	}
	else
	{
		dprintln("more data");
		onReceive(len, EtherSocket::chunk.raw);
	}

	return true;

}


uint16_t Socket::write(uint16_t len, const byte* data)
{
	return writeBuffer(len, data);
}


void Socket::processOutgoingBuffer()
{
	uint32_t nSeq = sequenceNumber;
	uint16_t dataLength;
	uint16_t dataChecksum;
	//dprint("processOutgoingBuffer, numSlots="); dprintln(numSlots);
	
	for (uint8_t n = numSlots; n > 0; n--)
	{
		dataLength = moveSlotToTXBuffer(sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(TCPHeader), n, dataChecksum);
		preparePacket(false, dataLength);
		EtherSocket::chunk.tcp.sequenceNumber.setValue(nSeq);
		EtherSocket::chunk.tcp.ACK = 1;

		dprint("dataLength="); dprint(dataLength);
		dprint(";dataChecksum="); dprintln(dataChecksum);

		calcTCPChecksum(false, dataLength, dataChecksum);

		EtherSocket::sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader));

		nSeq += dataLength;
	}

}

void Socket::releaseWindow(int32_t& bytesAck)
{
	uint16_t bytesReleased;
	
	do
	{
		bytesAck -= releaseFirstSlot();
	} while (bytesAck > 0 && numSlots>0);

	DEBUG(if (bytesAck < 0) dprintln("released too much ?!"));

}

void Socket::printState()
{
	char* s;

	switch (state)
	{
	case SCK_STATE_CLOSED: s = "CLOSED"; break;
	case SCK_STATE_LISTEN: s = "LISTEN"; break;
	case SCK_STATE_SYN_SENT: s = "SYN_SENT"; break;
	case SCK_STATE_SYN_RECEIVED: s = "SYN_RECEIVED"; break;
	case SCK_STATE_ESTABLISHED: s = "ESTABLISHED"; break;
	case SCK_STATE_FIN_WAIT_1: s = "FIN_WAIT_1"; break;
	case SCK_STATE_FIN_WAIT_2: s = "FIN_WAIT_2"; break;
	case SCK_STATE_CLOSE_WAIT: s = "CLOSE_WAIT"; break;
	case SCK_STATE_CLOSING: s = "CLOSING"; break;
	case SCK_STATE_LAST_ACK: s = "LAST_ACK"; break;
	case SCK_STATE_TIME_WAIT: s = "TIME_WAIT"; break;

	default:
		s = "UNKNOWN";
	}

	dprintln(s);

}