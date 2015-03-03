#ifndef _STATEFUL_H_
#define _STATEFUL_H_

#include <ACross.h>

class Stateful
{
protected:
	uint8_t stateTimer;
	uint8_t state;

	void setState(uint8_t newState, uint8_t timeout);

public :
	inline uint8_t getState()
	{
		return state;
	}

#if _DEBUG
	virtual __FlashStringHelper* getStateString();
#endif


};




#endif