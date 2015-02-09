#include "DNS.h"
#include "Checksum.h"

void DNSClient::onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data)
{


}

DNSClient::DNSClient() : identification(0)
{
	remotePort.setValue(53);
}

void DNSClient::setDNSAddress(const IPAddress& dnsServerIP)
{
	remoteAddress = dnsServerIP;
}
bool DNSClient::resolve(const char* name)
{

	uint8_t* b = chunk.raw;
	uint8_t* label = b;
	b++;
	
	for (*label = 0; *name != 0; name++,b++)
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

	uint16_t id = Checksum::calc(b - chunk.raw, chunk.raw);

	if (identification == 0)
	{

		DNSHeader header;
		header.zero();
		header.identification = id;
		/* 
		header.QR = 0; // query
		header.opcode = 0; // standard query;
		*/
		header.RD = 1; //recursion desired
		header.numberOfQuestions.setValue(1); //only one question
		b++;
		
		*((uint32_t*)b) = 0x01000100; //queryType = 1 (A query), queryClass=1 (Internet Address);
		
		b += 4;

		write(sizeof(header), (uint8_t*)&header);
		write(b - chunk.raw, chunk.raw);

		if (send())
			identification = id;


	}
	else
	{

	}


	return false;

}

