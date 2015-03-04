#ifndef _PING_H_
#define _PING_H_

#include <ACross.h>
#include "NetworkService.h"
#include "inet.h"

class Ping : public NetworkService
{
	bool onPacketReceived();


};

#endif