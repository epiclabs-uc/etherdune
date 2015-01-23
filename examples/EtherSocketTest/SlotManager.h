#ifndef _SLOTMANAGER_H_
#define _SLOTMANAGER_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "inet.h"
#include "config.h"



class SlotManager
{

private:

	uint16_t w;

	static uint16_t slotChecksum[NUM_SLOTS];
	static uint16_t slotData[NUM_SLOTS];

	union
	{
		struct
		{
			uint8_t slotId : 4;
			uint8_t slot2 : 4;
			uint8_t slot3 : 4;
			uint8_t slot4 : 4;

		};
		uint16_t slotQueue;
	};

	bool carry;
	bool odd;
	

	static uint8_t availableSlots;
	static uint16_t availableSlotBitmap;

	static uint8_t getSlot();
	static void freeSlot(uint8_t slotId);


protected:

	uint8_t numSlots;
	
	SlotManager();
	~SlotManager();


	
	uint16_t writeBuffer(uint16_t len, const uint8_t* data);
	uint16_t moveSlotToTXBuffer(uint16_t offset, uint8_t index, uint16_t& checksum);
	uint16_t releaseFirstSlot();

public:
};

#endif