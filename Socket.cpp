// 
// 
// 

#include "Socket.h"
#include "Checksum.h"

uint8_t Socket::srcPort_L_count = 0;


void Socket::onClose() {}
void Socket::onConnect() {}
void Socket::onReceive(uint16_t len, const byte* data) {}

Socket::Socket()
{
	
	state = SCK_STATE_CLOSED;
	sequenceNumber = 0;
	stateTimer = 0;
	sendAck = false;

	EtherFlow::registerSocket(this);
}

Socket::~Socket()
{
	EtherFlow::unregisterSocket(this);
}


void Socket::connect()
{
	
	randomSeed(millis() + analogRead(A1) + analogRead(A5));
	localPort.l =  random(255);//srcPort_L_count++;
	localPort.h = TCP_SRC_PORT_H;
	ackNumber = 0;

	setState(SCK_STATE_SYN_SENT,SCK_TIMEOUT_SYN_SENT);
	sendSYN();



}

void Socket::prepareIPPacket()
{
	EtherFlow::chunk.ip.version = 4;
	EtherFlow::chunk.ip.IHL = 0x05; //20 bytes
	EtherFlow::chunk.ip.raw[1] = 0x00; //DSCP/ECN=0;
	EtherFlow::chunk.ip.identification.setValue(0);
	EtherFlow::chunk.ip.flags = 0;
	EtherFlow::chunk.ip.fragmentOffset = 0;
	EtherFlow::chunk.ip.checksum.setValue(0);
	EtherFlow::chunk.ip.sourceIP = EtherFlow::localIP;
	EtherFlow::chunk.ip.destinationIP = remoteAddress;
	EtherFlow::chunk.ip.TTL = 255;
	EtherFlow::chunk.ip.checksum.rawu = ~Checksum::calc(sizeof(IPHeader), (uint8_t*)&EtherFlow::chunk.ip);
}

void Socket::prepareTCPPacket(bool options, uint16_t dataLength)
{
	
	EtherFlow::chunk.ip.totalLength.setValue(dataLength + (options ? 
		sizeof(IPHeader) + sizeof(TCPOptions) + sizeof(TCPHeader) :
		sizeof(IPHeader) + sizeof(TCPHeader)));

	EtherFlow::chunk.ip.protocol = IP_PROTO_TCP_V;

	prepareIPPacket();

	
	EtherFlow::chunk.tcp.sourcePort = localPort;
	EtherFlow::chunk.tcp.destinationPort = remotePort;
	EtherFlow::chunk.tcp.sequenceNumber.setValue(sequenceNumber);
	EtherFlow::chunk.tcp.flags = 0x00;

	EtherFlow::chunk.tcp.windowSize.setValue(512);
	EtherFlow::chunk.tcp.acknowledgementNumber.setValue(ackNumber);
	EtherFlow::chunk.tcp.checksum.zero();
	EtherFlow::chunk.tcp.urgentPointer.zero();

	EtherFlow::chunk.tcp.headerLength = options ? (sizeof(TCPHeader) + sizeof(TCPOptions)) / 4 : sizeof(TCPHeader) / 4;

	
}

uint16_t Socket::calcPseudoHeaderChecksum(uint8_t protocol, uint16_t length)
{
	nint32_t pseudo;
	pseudo.h.h = 0;
	pseudo.h.l = protocol;
	pseudo.l.setValue(length);

	uint16_t sum = Checksum::calc(sizeof(IPAddress) * 2, (uint8_t*)&EtherFlow::chunk.ip.sourceIP);
	return Checksum::calc(sum, sizeof(pseudo), (uint8_t*)&pseudo);
}

void Socket::calcTCPChecksum(bool options, uint16_t dataLength, uint16_t dataChecksum)
{
	
	uint8_t headerLength = options ? sizeof(TCPOptions) + sizeof(TCPHeader) : sizeof(TCPHeader);
	
	uint16_t sum = calcPseudoHeaderChecksum(IP_PROTO_TCP_V, dataLength + headerLength);
	sum = Checksum::calc(sum, headerLength, (uint8_t*)&EtherFlow::chunk.tcp);
	sum = Checksum::add(sum, dataChecksum);

	EtherFlow::chunk.tcp.checksum.rawu= ~sum;
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

	prepareTCPPacket(true,0);
	
	EtherFlow::chunk.tcp.SYN = 1;
	
	EtherFlow::chunk.tcpOptions.option1 = 0x02;
	EtherFlow::chunk.tcpOptions.option1_length = 0x04;
	EtherFlow::chunk.tcpOptions.option1_value.setValue(TCP_MAXIMUM_SEGMENT_SIZE);
	

	calcTCPChecksum(true, 0,0);

	EtherFlow::sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader) + sizeof(TCPOptions));
}

void Socket::sendFIN()
{
	dprintln("sendFIN()");

	prepareTCPPacket(false, 0);

	EtherFlow::chunk.tcp.FIN = 1;
	EtherFlow::chunk.tcp.ACK = 1;

	calcTCPChecksum(false, 0, 0);


	EtherFlow::sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader));
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
		dprint("SYN="); dprint(EtherFlow::chunk.tcp.SYN);	
		dprint(";ACK="); dprint(EtherFlow::chunk.tcp.ACK);
		dprint(";FIN="); dprint(EtherFlow::chunk.tcp.FIN);
		dprint(";RST="); dprintln(EtherFlow::chunk.tcp.RST);

		uint32_t incomingAckNum = EtherFlow::chunk.tcp.acknowledgementNumber.getValue();
		uint32_t incomingSeqNum = EtherFlow::chunk.tcp.sequenceNumber.getValue();

		int32_t bytesAck = (int32_t)(incomingAckNum - sequenceNumber);
		int32_t bytesReceived; // = (int32_t)(incomingSeqNum - ackNumber);

		dprint("incomingAck="); dprintln(incomingAckNum);
		dprint("localSeqNum="); dprintln(sequenceNumber);
		dprint("bytesAck="); dprintln(bytesAck);

		dprint("incomingSeqNum="); dprintln(incomingSeqNum);
		dprint("localAckNum="); dprintln(ackNumber);

		if (EtherFlow::chunk.tcp.RST)
		{
			terminate();
			return false;
		}

		if (bytesAck >0)
			sequenceNumber += bytesAck;


		if (state == SCK_STATE_SYN_SENT && EtherFlow::chunk.tcp.SYN && EtherFlow::chunk.tcp.ACK)
		{
			ackNumber = incomingSeqNum + 1;
			setState(SCK_STATE_ESTABLISHED,0);
			onConnect();
			sendAck = true;
			return false;
		}

		if (ackNumber != incomingSeqNum)
		{
			dprintln("dropped packet out of sequence.");
			sendAck = true;
			return false;
		}

		uint16_t headerLength = sizeof(IPHeader) + EtherFlow::chunk.tcp.headerLength * 4;
		bytesReceived = EtherFlow::chunk.ip.totalLength.getValue() - headerLength;
		ackNumber += bytesReceived;
		dprint("bytesReceived="); dprintln(bytesReceived);

		releaseWindow(bytesAck);
		sendAck = true;
		

		if (EtherFlow::chunk.tcp.FIN)
			ackNumber++;


		switch (state)
		{
			case SCK_STATE_ESTABLISHED:
			{
				uint16_t slen =(uint16_t) min(bytesReceived, sizeof(EthBuffer) - sizeof(EthernetHeader) - headerLength);
				
				if (slen>0)
					onReceive(slen, EtherFlow::chunk.raw + sizeof(EthernetHeader) + headerLength);

				if (EtherFlow::chunk.tcp.FIN)
				{
					setState(SCK_STATE_CLOSE_WAIT, 0);
					//sendFIN();
					onClose();
				}


			}break;


			case SCK_STATE_FIN_WAIT_1:
			{
				if (EtherFlow::chunk.tcp.FIN)
					setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);
				else
					setState(SCK_STATE_FIN_WAIT_2, SCK_TIMEOUT_FIN_WAIT_2);

			}break;
			case SCK_STATE_FIN_WAIT_2:
			{
				if (EtherFlow::chunk.tcp.FIN)
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
		onReceive(len, EtherFlow::chunk.raw);
	}

	return true;

}


uint16_t Socket::write(uint16_t len, const byte* data)
{
	return SharedBuffer::write(len, data);
}

uint16_t Socket::send(uint16_t len, const byte* data)
{
	EtherFlow::writeBuf(TXSTART_INIT_DATA + sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(UDPHeader), len, data);


	EtherFlow::chunk.ip.totalLength.setValue(len + sizeof(IPHeader) + sizeof(UDPHeader));
	EtherFlow::chunk.ip.protocol = IP_PROTO_UDP_V;
	prepareIPPacket();

	EtherFlow::chunk.udp.sourcePort = localPort;
	EtherFlow::chunk.udp.destinationPort = remotePort;
	EtherFlow::chunk.udp.dataLength.setValue(len+sizeof(UDPHeader));
	EtherFlow::chunk.udp.checksum.zero();

	uint16_t checksum = calcPseudoHeaderChecksum(IP_PROTO_UDP_V, len + sizeof(UDPHeader));
	checksum = Checksum::calc(checksum, sizeof(UDPHeader), (uint8_t*)&EtherFlow::chunk.udp);

	EtherFlow::chunk.udp.checksum.rawu = ~ Checksum::calc(checksum, len , data);

	EtherFlow::sendIPPacket(sizeof(IPHeader) + sizeof(UDPHeader));

	return len;
}


void Socket::processOutgoingBuffer()
{
	uint32_t nSeq = sequenceNumber;
	uint16_t dataLength;
	uint16_t dataChecksum;
	//dprint("processOutgoingBuffer, numSlots="); dprintln(numSlots);
	
	if (next != 0xFFFF || sendAck)
	{
		sendAck = false;
		dataLength = fillTxBuffer(sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(TCPHeader), dataChecksum);
		prepareTCPPacket(false, dataLength);
		EtherFlow::chunk.tcp.sequenceNumber.setValue(nSeq);
		EtherFlow::chunk.tcp.ACK = 1;

		dprint("dataLength="); dprint(dataLength);
		dprint(";dataChecksum="); dprintln(dataChecksum);

		calcTCPChecksum(false, dataLength, dataChecksum);

		EtherFlow::sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader));

		nSeq += dataLength;

	}

}

void Socket::releaseWindow(int32_t& bytesAck)
{
	
	while (bytesAck > 0 && next != 0xFFFF)
	{
		bytesAck -= release();
	} ;

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