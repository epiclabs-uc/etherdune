#include "Stateful.h"


#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("Stateful");

#if _DEBUG
__FlashStringHelper* Stateful::getStateString() { return (__FlashStringHelper*)PSTR("UNKNOWN"); };
#endif

void Stateful::setState(uint8_t newState, uint8_t timeout)
{
	state = newState;
	stateTimer = timeout;

	ACDEBUG("set state=%S", getStateString());
}