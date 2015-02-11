#include "DNS.h"
#include "Checksum.h"

DEFINE_FLOWPATTERN(catchDNSResponse, "%*[\0]\x01" "%*[\0]\x01" "%*4c" "%*[\0]\x04" "%4c");

void DNSClient::onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data)
{
	if (datagramLength != 0)
	{
		scanner.reset();
		receivedId = chunk.dns.identification;
	}

	while (fragmentLength--)
	{
		if (scanner.scan(*data, &resolvedIP))
		{
			identification = receivedId;
		}

		data++;
	}
}

DNSClient::DNSClient() : identification(0), timer(0)
{
	remotePort.setValue(53);
	scanner.setPattern(catchDNSResponse);
}

bool DNSClient::resolve(const char* name)
{

	uint8_t* b = chunk.raw;
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

	uint16_t id = Checksum::calc(b - chunk.raw, chunk.raw);

	if (identification == id)
		return true;

	if (timer == 0)
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

		//queryType = 1 (A query), queryClass=1 (Internet Address);
		b++;
		*((uint32_t*)b) = 0x01000100; //queryType = 1 (A query), queryClass=1 (Internet Address);


		b += 4;

		write(sizeof(header), (uint8_t*)&header);
		write(b - chunk.raw, chunk.raw);

		if (send())
			timer = DNS_TIMEOUT_QUERY;
	}

	return false;
}

void DNSClient::tick()
{
	if (timer > 0)
		timer--;

	UDPSocket::tick();
}

