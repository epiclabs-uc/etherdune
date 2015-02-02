#include "SharedBuffer.h"

#include "EtherFlow.h"
#include "Checksum.h"

uint16_t SharedBuffer::head = 0;
uint16_t SharedBuffer::usedSpace = 0;


SharedBuffer::SharedBuffer() :next(0xFFFF), lastWritten(0xFFFF)
{

}


uint16_t SharedBuffer::writeAt(uint16_t index, uint16_t len, const byte* data)
{
	// Write in a single step
	if (len <= SHARED_BUFFER_CAPACITY - index)
	{
		EtherFlow::writeBuf(SHARED_BUFFER_INIT + index, len, data);
		index += len;
		if (index == SHARED_BUFFER_CAPACITY) index = 0;


	}
	// Write in two steps
	else
	{
		uint16_t size_1 = SHARED_BUFFER_CAPACITY - index;
		EtherFlow::writeBuf(SHARED_BUFFER_INIT + index, size_1, data);
		
		uint16_t size_2 = len - size_1;
		EtherFlow::writeBuf(SHARED_BUFFER_INIT , size_2, data+size_1);
		
		index = size_2;
	}


	return index;

}

uint16_t SharedBuffer::append(uint16_t len, const byte* data)
{
	if (len == 0) return 0;

	uint16_t bytes_to_write = min(len, SHARED_BUFFER_CAPACITY - usedSpace);

	head = writeAt(head,bytes_to_write,data);

	usedSpace += bytes_to_write;
	
	return usedSpace;
}

uint16_t SharedBuffer::readAt(uint16_t index, uint16_t len, byte* data)
{
	// Read in a single step
	if (len <= SHARED_BUFFER_CAPACITY - index)
	{
		EtherFlow::readBuf(SHARED_BUFFER_INIT + index, len, data);
		index += len;
		if (index == SHARED_BUFFER_CAPACITY) index = 0;
	}
	// Read in two steps
	else
	{
		uint16_t size_1 = SHARED_BUFFER_CAPACITY - index;
		EtherFlow::readBuf(SHARED_BUFFER_INIT + index, size_1, data);
		
		uint16_t size_2 = len - size_1;
		EtherFlow::readBuf(SHARED_BUFFER_INIT, size_2, data + size_1);
		
		index = size_2;
	}


	return index;
}


uint16_t SharedBuffer::write(uint16_t len, const byte* data)
{
	int16_t availableSpace = SHARED_BUFFER_CAPACITY - usedSpace - sizeof(BufferHeader);
	if (availableSpace < 0)
		availableSpace = 0;

	len = min(len, availableSpace);

	if (len == 0)
		return len;

	BufferHeader header;
	header.length = len;
	header.checksum = Checksum::calc(len,data);

	header.nextIndex = 0xFFFF;

	uint16_t h = head;
	append(sizeof(header), (byte*)&header);
	append(len, data);
	

	if (lastWritten != 0xFFFF)
		writeAt(lastWritten, sizeof(h), (byte*)&h);
	else
		next = h;

	lastWritten = h;

	return len;
}

uint16_t SharedBuffer::release()
{
	if (next == 0xFFFF)
		return 0;

	BufferHeader header;

	readAt(next, sizeof(header), (byte*) &header);
	
	next = header.nextIndex;

	uint16_t usedSpace=0;

	for (uint8_t i = 0; i < MAX_TCP_SOCKETS; i++)
	{
		SharedBuffer* s = (SharedBuffer*)EtherFlow::sockets[i];

		if (s == NULL || s->next==0xFFFF)
			continue;

		uint16_t d = (s->next < head) ? head - s->next : SHARED_BUFFER_CAPACITY - s->next + head;
		
		if (d > usedSpace)
			usedSpace = d;
	}

	return header.length;

}


uint16_t SharedBuffer::fillTxBuffer(uint16_t dstOffset, uint16_t& checksum )
{

	BufferHeader header;
	checksum = 0;

	uint16_t txPtr = TXSTART_INIT_DATA + dstOffset;
	bool startOdd = txPtr & 1;

	uint16_t n = next;
	while (readAt(n, sizeof(header), (byte*)&header), n!=0xFFFF && (txPtr + header.length <= TXSTOP_INIT+1))
	{

		EtherFlow::moveMem(txPtr, SHARED_BUFFER_INIT + n + sizeof(header), header.length);

		checksum = Checksum::add(checksum, header.checksum, startOdd ^ (txPtr & 1));

		txPtr += header.length;
		n = header.nextIndex;

	}

	return txPtr - (TXSTART_INIT_DATA + dstOffset);
}