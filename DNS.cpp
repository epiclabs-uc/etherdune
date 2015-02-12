#include "DNS.h"
#include "Checksum.h"

DNSQuery::DNSQuery() 
{
	resolvedIP.u = 0;
	name = NULL;
}


void DNSQuery::resolve(char* hostName)
{
	resolvedIP.u = 0; //also sets to 0 "flag", "identification" and "timer"
	timer = DNS_TIMEOUT_QUERY;
	name = hostName;

}

uint8_t DNSQuery::getResponse(IPAddress& ip)
{
	uint8_t status;
	if (flag == 0)
	{
		if (timer == 0)
			status = 2;
		else
			status = 1;
	}
	else
	{
		ip = resolvedIP;
		status = 0;
	}
	return status;
}




DEFINE_FLOWPATTERN(catchDNSResponse, "%*[\0]\x01" "%*[\0]\x01" "%*4c" "%*[\0]\x04" "%4c");

bool DNSClient::onReceive(uint16_t fragmentLength, uint16_t datagramLength, const byte* data)
{
	if (datagramLength != 0) // it is the first chunk of this datagram
	{
		for (DNSQuery* query = (DNSQuery*)queryList.first; query != NULL; query = (DNSQuery*)query->nextItem)
		{
			if (query->identification == chunk.dns.identification)
			{
				scanner.reset();
				selectedQuery = query;
				return true; //bring in the rest of this datagram
			}
		}
		return false; // not interested in the rest of this datagram.
	}

	
	while (fragmentLength--)
	{
		if (scanner.scan(*data, &resolvedIP))
		{
			selectedQuery->resolvedIP = resolvedIP;
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
		return id;
	else
		return 0;
}


void DNSClient::tick()
{
	for (DNSQuery* query = (DNSQuery*)queryList.first; query != NULL; query = (DNSQuery*)query->nextItem)
	{
		if (query->flag == 0 && query->name != NULL)
		{
			if (query->timer > 0)
			{
				query->timer--;
				uint16_t id = resolve(query->name);
				if (query->identification == 0)
					query->identification = id;
			}
		}

	}

	UDPSocket::tick();
}

void DNSClient::addQuery(DNSQuery& query)
{
	queryList.add(&query);
}

void DNSClient::removeQuery(DNSQuery& query)
{
	queryList.remove(&query);
}