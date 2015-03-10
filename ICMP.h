#ifndef _ICMP_H_
#define _ICMP_H_

#include <ACross.h>
#include "NetworkService.h"

class ICMP : protected NetworkService
{

private:
	
	bool onPacketReceived();
	void calcICMPChecksum();

protected:

	virtual void onPingReply(uint16_t time);

public:

	void ping(const IPAddress& targetIP);

};

#endif