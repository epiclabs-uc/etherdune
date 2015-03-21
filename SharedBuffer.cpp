// EtherDune Shared circular buffer class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements a "shared" circular buffer using spare ENC28J60 memory
//
// Copyright (c) 2015 All Rights Reserved, http://friendev.com
//
// This source is subject to the GPLv2 license.
// Please see the License.txt file for more information.
// All other rights reserved.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.

#include "SharedBuffer.h"

#include "ENC28J60.h"
#include "Checksum.h"

#define AC_LOGLEVEL 2
#include <ACLog.h>
ACROSS_MODULE("SharedBuffer");

struct BufferHeader
{
	uint16_t nextIndex;
	uint16_t length;
	uint16_t checksum;
};

uint16_t SharedBuffer::head = 0;
uint16_t SharedBuffer::usedSpace = 0;

SharedBuffer::SharedBuffer() :nextRead(0xFFFF), lastWritten(0xFFFF)
{
	bufferList.add(this);
}

SharedBuffer::~SharedBuffer()
{
	flush();
	bufferList.remove(this);
}


uint16_t SharedBuffer::writeAt(uint16_t index, uint16_t len, const byte* data)
{
	// Write in a single step
	if (len <= SHARED_BUFFER_CAPACITY - index)
	{
		ENC28J60::writeBuf(SHARED_BUFFER_INIT + index, len, data);
		index += len;
		if (index == SHARED_BUFFER_CAPACITY) 
			index = 0;


	}
	// Write in two steps
	else
	{
		uint16_t size_1 = SHARED_BUFFER_CAPACITY - index;
		ENC28J60::writeBuf(SHARED_BUFFER_INIT + index, size_1, data);
		
		uint16_t size_2 = len - size_1;
		ENC28J60::writeBuf(SHARED_BUFFER_INIT , size_2, data+size_1);
		
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
	
	return bytes_to_write;
}

uint16_t SharedBuffer::readAt(uint16_t index, uint16_t len, byte* data)
{
	// Read in a single step
	if (len <= SHARED_BUFFER_CAPACITY - index)
	{
		ENC28J60::readBuf(SHARED_BUFFER_INIT + index, len, data);
		index += len;
		if (index == SHARED_BUFFER_CAPACITY) index = 0;
	}
	// Read in two steps
	else
	{
		uint16_t size_1 = SHARED_BUFFER_CAPACITY - index;
		ENC28J60::readBuf(SHARED_BUFFER_INIT + index, size_1, data);
		
		uint16_t size_2 = len - size_1;
		ENC28J60::readBuf(SHARED_BUFFER_INIT, size_2, data + size_1);
		
		index = size_2;
	}


	return index;
}


/// <summary>
/// Writes a fragment to the shared buffer
/// </summary>
/// <param name="len">length of the data pointed by `data`</param>
/// <param name="data">pointer to the data to write</param>
/// <returns>Number of bytes actually written</returns>
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
		nextRead = h;

	lastWritten = h;

	return len;
}

/// <summary>
/// Releases one fragment of data
/// </summary>
/// <returns>The amount of data released</returns>
uint16_t SharedBuffer::release()
{
	if (isEmpty())
		return 0;

	BufferHeader header;

	readAt(nextRead, sizeof(header), (byte*) &header);
	
	nextRead = header.nextIndex;

	if (isEmpty())
		lastWritten = 0xFFFF;

	usedSpace=0;

	for (SharedBuffer* s = (SharedBuffer*)bufferList.first; s->nextItem != NULL; s = (SharedBuffer*)s->nextItem)
	{
		if (s->isEmpty())
			continue;

		uint16_t d = (s->nextRead < head) ? head - s->nextRead : SHARED_BUFFER_CAPACITY - s->nextRead + head;

		if (d > usedSpace)
			usedSpace = d;

	}

	ACTRACE("release(). head=%d, usedSpace=%d", head,usedSpace);
	return header.length;

}

/// <summary>
/// Releases all data in this buffer, freeing up space.
/// </summary>
void SharedBuffer::flush()
{
	while (release());
}


/// <summary>
/// Copies up to `count` fragments to the transmit buffer via DMA, concatenating them starting at given offset.
/// This function may copy fewer fragments than `count` to make sure not to overflow the TX buffer
/// </summary>
/// <param name="dstOffset">Destination %ENC28J60 memory address, encoded as an offset within the tx buffer.</param>
/// <param name="checksum">[out] Resulting checksum of copied data</param>
/// <param name="count">Maximum number of fragments to concatenate</param>
/// <returns>Total number of bytes copied to the tx buffer</returns>
uint16_t SharedBuffer::fillTxBuffer(uint16_t dstOffset, uint16_t& checksum, uint16_t count )
{

	BufferHeader header;
	checksum = 0;

	uint16_t txPtr = TXSTART_INIT_DATA + dstOffset;
	bool startOdd = txPtr & 1;

	uint16_t n = nextRead;
	while (n!=0xFFFF && count>0)
	{
		count--;
		readAt(n, sizeof(header), (byte*)&header);
		if ((txPtr + header.length > TXSTOP_INIT + 1))
			break;
		
		uint16_t src = SHARED_BUFFER_INIT + n + sizeof(header);

		if (src >= SHARED_BUFFER_INIT + SHARED_BUFFER_CAPACITY)
			src -= SHARED_BUFFER_CAPACITY;

		uint16_t len = header.length;
		bool odd = startOdd ^ (txPtr & 1);

		if (src + len > SHARED_BUFFER_INIT + SHARED_BUFFER_CAPACITY)
		{
			len = SHARED_BUFFER_INIT + SHARED_BUFFER_CAPACITY - src; 
			ENC28J60::moveMem(txPtr, src, len);
			txPtr += len;
			len = header.length - len; 
			src = SHARED_BUFFER_INIT;
		}

		ENC28J60::moveMem(txPtr, src,len);

		checksum = Checksum::add(checksum, header.checksum, odd);

		txPtr += len;
		n = header.nextIndex;

	}

	return txPtr - (TXSTART_INIT_DATA + dstOffset);
}


/// <summary>
/// Determines whether this buffer is empty
/// </summary>
/// <returns>`true` if the buffer is empty. `false` otherwise</returns>
bool SharedBuffer::isEmpty()
{
	return nextRead == 0xFFFF;
}
