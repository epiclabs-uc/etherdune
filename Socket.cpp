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

	
}




void Socket::connect()
{
	randomSeed((uint16_t)millis() + analogRead(A1) + analogRead(A5));
	localPort.l = random(255); //srcPort_L_count++;
	localPort.h = TCP_SRC_PORT_H;
	ackNumber = 0;

	setState(SCK_STATE_SYN_SENT,SCK_TIMEOUT_SYN_SENT);
	sendSYN();



}

void Socket::prepareIPPacket()
{
	chunk.ip.version = 4;
	chunk.ip.IHL = 0x05; //20 bytes
	chunk.ip.raw[1] = 0x00; //DSCP/ECN=0;
	chunk.ip.identification.setValue(0);
	chunk.ip.flags = 0;
	chunk.ip.fragmentOffset = 0;
	chunk.ip.checksum.setValue(0);
	chunk.ip.sourceIP = localIP;
	chunk.ip.destinationIP = remoteAddress;
	chunk.ip.TTL = 255;
	chunk.ip.checksum.rawu = ~Checksum::calc(sizeof(IPHeader), (uint8_t*)&chunk.ip);
}

void Socket::prepareTCPPacket(bool options, uint16_t dataLength)
{
	
	chunk.ip.totalLength.setValue(dataLength + (options ? 
		sizeof(IPHeader) + sizeof(TCPOptions) + sizeof(TCPHeader) :
		sizeof(IPHeader) + sizeof(TCPHeader)));

	chunk.ip.protocol = IP_PROTO_TCP_V;

	prepareIPPacket();

	
	chunk.tcp.sourcePort = localPort;
	chunk.tcp.destinationPort = remotePort;
	chunk.tcp.sequenceNumber.setValue(sequenceNumber);
	chunk.tcp.flags = 0x00;

	chunk.tcp.windowSize.setValue(512);
	chunk.tcp.acknowledgementNumber.setValue(ackNumber);
	chunk.tcp.checksum.zero();
	chunk.tcp.urgentPointer.zero();

	chunk.tcp.headerLength = options ? (sizeof(TCPHeader) + sizeof(TCPOptions)) / 4 : sizeof(TCPHeader) / 4;

	
}

uint16_t Socket::calcPseudoHeaderChecksum(uint8_t protocol, uint16_t length)
{
	nint32_t pseudo;
	pseudo.h.h = 0;
	pseudo.h.l = protocol;
	pseudo.l.setValue(length);

	uint16_t sum = Checksum::calc(sizeof(IPAddress) * 2, (uint8_t*)&chunk.ip.sourceIP);
	return Checksum::calc(sum, sizeof(pseudo), (uint8_t*)&pseudo);
}

void Socket::calcTCPChecksum(bool options, uint16_t dataLength, uint16_t dataChecksum)
{
	
	uint8_t headerLength = options ? sizeof(TCPOptions) + sizeof(TCPHeader) : sizeof(TCPHeader);
	
	uint16_t sum = calcPseudoHeaderChecksum(IP_PROTO_TCP_V, dataLength + headerLength);
	sum = Checksum::calc(sum, headerLength, (uint8_t*)&chunk.tcp);
	sum = Checksum::add(sum, dataChecksum);

	chunk.tcp.checksum.rawu= ~sum;
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
	
	chunk.tcp.SYN = 1;
	
	chunk.tcpOptions.option1 = 0x02;
	chunk.tcpOptions.option1_length = 0x04;
	chunk.tcpOptions.option1_value.setValue(TCP_MAXIMUM_SEGMENT_SIZE);
	

	calcTCPChecksum(true, 0,0);

	sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader) + sizeof(TCPOptions));
}

void Socket::sendFIN()
{
	dprintln("sendFIN()");

	prepareTCPPacket(false, 0);

	chunk.tcp.FIN = 1;
	chunk.tcp.ACK = 1;

	calcTCPChecksum(false, 0, 0);


	sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader));
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

bool Socket::processHeader()
{
	if (!(
		chunk.eth.etherType.getValue() == ETHTYPE_IP &&
		chunk.ip.protocol == IP_PROTO_TCP_V &&
		state != SCK_STATE_CLOSED &&
		localPort.rawu == chunk.tcp.destinationPort.rawu))
	{
		return false;
	}

	dprintln("Header");
	dprint("SYN="); dprint(chunk.tcp.SYN);
	dprint(";ACK="); dprint(chunk.tcp.ACK);
	dprint(";FIN="); dprint(chunk.tcp.FIN);
	dprint(";RST="); dprintln(chunk.tcp.RST);

	uint32_t incomingAckNum = chunk.tcp.acknowledgementNumber.getValue();
	uint32_t incomingSeqNum = chunk.tcp.sequenceNumber.getValue();

	int32_t bytesAck = (int32_t)(incomingAckNum - sequenceNumber);
	int32_t bytesReceived; // = (int32_t)(incomingSeqNum - ackNumber);

	dprint("incomingAck="); dprintln(incomingAckNum);
	dprint("localSeqNum="); dprintln(sequenceNumber);
	dprint("bytesAck="); dprintln(bytesAck);

	dprint("incomingSeqNum="); dprintln(incomingSeqNum);
	dprint("localAckNum="); dprintln(ackNumber);

	if (chunk.tcp.RST)
	{
		terminate();
		return false;
	}

	if (bytesAck > 0)
		sequenceNumber += bytesAck;


	if (state == SCK_STATE_SYN_SENT && chunk.tcp.SYN && chunk.tcp.ACK)
	{
		ackNumber = incomingSeqNum + 1;
		setState(SCK_STATE_ESTABLISHED, 0);
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

	int16_t headerLength = sizeof(IPHeader) + chunk.tcp.headerLength * 4;
	bytesReceived = chunk.ip.totalLength.getValue() - headerLength;
	ackNumber += bytesReceived;
	dprint("bytesReceived="); dprintln(bytesReceived);

	releaseWindow(bytesAck);
	sendAck = true;


	if (chunk.tcp.FIN)
		ackNumber++;


	switch (state)
	{
		case SCK_STATE_ESTABLISHED:
		{
			int16_t slen = min(bytesReceived, (int16_t)(sizeof(EthBuffer) - sizeof(EthernetHeader)) - headerLength);

			if (slen > 0)
				processData((uint16_t)slen, chunk.raw + sizeof(EthernetHeader) + headerLength);
				

			if (chunk.tcp.FIN)
			{
				setState(SCK_STATE_CLOSE_WAIT, 0);
				//sendFIN();
				onClose();
			}


		}break;


		case SCK_STATE_FIN_WAIT_1:
		{
			if (chunk.tcp.FIN)
				setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);
			else
				setState(SCK_STATE_FIN_WAIT_2, SCK_TIMEOUT_FIN_WAIT_2);

		}break;
		case SCK_STATE_FIN_WAIT_2:
		{
			if (chunk.tcp.FIN)
				setState(SCK_STATE_TIME_WAIT, SCK_TIMEOUT_TIME_WAIT);

		}break;

		case SCK_STATE_LAST_ACK:
		{
			setState(SCK_STATE_CLOSED, 0);
		}


		default:
			break;
	}






	return true;
}

bool Socket::processData(uint16_t len, uint8_t* data)
{
	dprintln("more data");

	onReceive(len, data);

	return true;

}


uint16_t Socket::write(uint16_t len, const byte* data)
{
	return buffer.write(len, data);
}

uint16_t Socket::send(uint16_t len, const byte* data)
{
	EtherFlow::writeBuf(TXSTART_INIT_DATA + sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(UDPHeader), len, data);


	chunk.ip.totalLength.setValue(len + sizeof(IPHeader) + sizeof(UDPHeader));
	chunk.ip.protocol = IP_PROTO_UDP_V;
	prepareIPPacket();

	chunk.udp.sourcePort = localPort;
	chunk.udp.destinationPort = remotePort;

	chunk.udp.dataLength.setValue(len+sizeof(UDPHeader));
	chunk.udp.checksum.zero();

	uint16_t checksum = calcPseudoHeaderChecksum(IP_PROTO_UDP_V, len + sizeof(UDPHeader));
	checksum = Checksum::calc(checksum, sizeof(UDPHeader), (uint8_t*)&chunk.udp);

	chunk.udp.checksum.rawu = ~ Checksum::calc(checksum, len , data);

	sendIPPacket(sizeof(IPHeader) + sizeof(UDPHeader));

	return len;
}


void Socket::processOutgoingBuffer()
{
	uint32_t nSeq = sequenceNumber;
	uint16_t dataLength;
	uint16_t dataChecksum;
	//dprint("processOutgoingBuffer, numSlots="); dprintln(numSlots);
	
	if (buffer.nextRead != 0xFFFF || sendAck)
	{
		sendAck = false;
		dataLength = buffer.fillTxBuffer(sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(TCPHeader), dataChecksum);
		prepareTCPPacket(false, dataLength);
		chunk.tcp.sequenceNumber.setValue(nSeq);
		chunk.tcp.ACK = 1;

		dprint("dataLength="); dprint(dataLength);
		dprint(";dataChecksum="); dprintln(dataChecksum);

		calcTCPChecksum(false, dataLength, dataChecksum);

		sendIPPacket(sizeof(IPHeader) + sizeof(TCPHeader));

		nSeq += dataLength;

	}

}

void Socket::releaseWindow(int32_t& bytesAck)
{
	
	while (bytesAck > 0 && buffer.nextRead != 0xFFFF)
	{
		bytesAck -= buffer.release();
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