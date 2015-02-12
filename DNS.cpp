#include "DNS.h"
#include "Checksum.h"


DEFINE_FLOWPATTERN(catchDNSResponse, "%*[\0]\x01" "%*[\0]\x01" "%*4c" "%*[\0]\x04" "%4c");

void DNSClient::onResolve(uint16_t id, const IPAddress& ip)
{
	NetworkService::notifyOnDNSResolve(id, ip);
}

bool DNSClient::onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data)
{
	if (datagramLength != 0) // it is the first chunk of this datagram
	{
		identification = chunk.dns.identification;
		return true; //bring in the rest of this datagram
	}

	
	while (fragmentLength--)
	{
		if (scanner.scan(*data, &resolvedIP))
		{
			onResolve(identification, resolvedIP);
			buffer.release();
			return false;
		}

		data++;
	}
	return true; // continue processing the next chunk
}

DNSClient::DNSClient() 
{
	remotePort.setValue(53);
	scanner.setPattern(catchDNSResponse);
}

uint16_t DNSClient::resolve(const char* name)
{
	timer = DNS_TIMEOUT_QUERY;

	DNSHeader& header = *(DNSHeader*)chunk.raw;

	uint8_t* queryPtr = chunk.raw + sizeof(DNSHeader);
	uint8_t* b = queryPtr;
	uint8_t* label = b;
	b++;

	for (*label = 0; *name != 0; name++, b++)
	{
		if (*name == '.')
		{
			label = b;

			*label = 0;
		}
		else
		{
			*b = *name;
			(*label)++;
		}
	}

	*b = 0;

	uint16_t id = Checksum::calc(b - queryPtr, queryPtr);

	header.zero();
	header.identification = id;
	/*
	header.QR = 0; // query
	header.opcode = 0; // standard query;
	*/
	header.RD = 1; //recursion desired
	header.numberOfQuestions.setValue(1); //only one question

	//queryType = 1 (A query), queryClass=1 (Internet Address);
	b++;
	*((uint32_t*)b) = 0x01000100; //queryType = 1 (A query), queryClass=1 (Internet Address);


	b += 4;

	write(sizeof(header) + b - queryPtr, chunk.raw);

	send();
	return id;
}


void DNSClient::tick()
{
	if (timer > 0)
	{
		timer--;

		if (timer == 0)
		{
			buffer.release();
			timer = DNS_TIMEOUT_QUERY;
		}
	}


	UDPSocket::tick();
}

bool DNSClient::sendPacket()
{
	uint16_t dataChecksum = 0;
	uint16_t dataLength;

	dataLength = buffer.fillTxBuffer(sizeof(EthernetHeader) + sizeof(IPHeader) + sizeof(UDPHeader),/*out*/ dataChecksum, 1);

	bool ret;
	if (dataLength > 0)
	{
		prepareUDPPacket(dataLength, dataChecksum);
		ret = !sendIPPacket(sizeof(IPHeader) + sizeof(UDPHeader));
	}
	else
		ret = false;
	
	return ret;

}