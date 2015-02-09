#ifndef SHARED_BUFFER_H
#define SHARED_BUFFER_H



#include "config.h"
#include "List.h"

struct BufferHeader
{
	uint16_t nextIndex;
	uint16_t length;
	uint16_t checksum;
};


class SharedBuffer : ListItem
{
	
	static uint16_t head;
	static uint16_t usedSpace;

	static uint16_t append(uint16_t len, const byte* data);
	static uint16_t writeAt(uint16_t index, uint16_t len, const byte* data);
	static uint16_t readAt(uint16_t index, uint16_t len, byte* data );

	static List bufferList;

public:
	SharedBuffer* next;


	uint16_t nextRead;
	uint16_t lastWritten;
	
	uint16_t write(uint16_t len, const byte* data);
	uint16_t release();
	void flush();

	uint16_t fillTxBuffer(uint16_t dstOffset, uint16_t& checksum);


public:
	SharedBuffer();
	~SharedBuffer();



};







#endif