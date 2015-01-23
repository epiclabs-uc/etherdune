#include "SlotManager.h"
#include "ethernet.h"


uint8_t SlotManager::availableSlots = NUM_SLOTS;
uint16_t SlotManager::availableSlotBitmap = 0;
uint16_t SlotManager::slotChecksum[NUM_SLOTS] = {};
uint16_t SlotManager::slotData[NUM_SLOTS] = {};


SlotManager::SlotManager()
{
	w = SLOT_SIZE;
	numSlots = 0;
	carry = false;
	odd = false;
}

SlotManager::~SlotManager()
{
	while (numSlots > 0)
		releaseFirstSlot();
}


uint8_t SlotManager::getSlot()
{

	uint16_t mask = 1;
	for (uint8_t i = 0; i < NUM_SLOTS; i++, mask <<= 1)
	{
		if ((availableSlotBitmap & mask) == 0)
		{
			availableSlots--;
			availableSlotBitmap |= mask;
			return i;
		}
	}

	return 0xFF; // no more slots available.
}

void SlotManager::freeSlot(uint8_t slotId)
{
	uint16_t mask = 1 << slotId;

	if (availableSlotBitmap & mask)
	{
		availableSlots++;
		mask = ~mask;
		availableSlotBitmap &= mask;

	}
}



uint16_t SlotManager::writeBuffer(uint16_t len, const uint8_t* data)
{
	dprint("writeBuffer.w="); dprintln(w);
	uint8_t s;
	uint8_t* ptr = (uint8_t*) data;
		
	do
	{

		if (w == SLOT_SIZE)
		{
			if (numSlots < MAX_SLOTS_PER_SOCKET && ((s = getSlot()) != 0xFF))
			{
				dprint("s="); dprintln(s);
				slotQueue <<= 4;
				slotId = s;
				slotChecksum[slotId] = 0;
				w = 0;
				carry = 0;
				odd = 0;
				numSlots++;
			}
			else
				return len;
		}


		uint16_t slen = min(SLOT_SIZE - w, len);

		EtherSocket::writeBuf(SLOT_ADDR(slotId) + w, slen, ptr);
		w += slen;
		
		slotChecksum[slotId] = EtherSocket::checksum(slotChecksum[slotId], ptr, slen, carry, odd);
		slotData[slotId] = w;
		ptr += slen;
		len -= slen;

	} while ((numSlots <= MAX_SLOTS_PER_SOCKET) && len > 0);

	return len;

}

uint16_t SlotManager::moveSlotToTXBuffer(uint16_t offset, uint8_t index, uint16_t& checksum)
{
	uint8_t s;
	uint16_t dataMoved = 0;

	int8_t i = 4*(index - 1); 

	if(i >= 0) 
	{
		s = (slotQueue & (0x0F << i )) >> i ;

		dataMoved = slotData[s];
		if (i == 0)
			w = SLOT_SIZE;

		EtherSocket::moveMem(TXSTART_INIT_DATA + offset, SLOT_ADDR(s), dataMoved);

		checksum = slotChecksum[s];

	}

	return dataMoved;
}

uint16_t SlotManager::releaseFirstSlot()
{
	uint8_t s;
	uint16_t dataReleased = 0;
	
	int8_t i = 4 * (numSlots - 1);

	if (i>=0)
	{
		s = (slotQueue & (0x0F << i)) >> i;
		
		dataReleased = slotData[s];
		numSlots--;
		freeSlot(s);
	}

	if (numSlots == 0)
		w = SLOT_SIZE;
	
	dprint("dataReleased="); dprintln(dataReleased);
	
	return dataReleased;

}

