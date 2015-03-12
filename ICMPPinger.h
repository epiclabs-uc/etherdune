#ifndef _ICMPPINGER_H_
#define ICMPPINGER_H_

#include "ICMP.h"


class ICMPPinger : protected ICMP
{
	
	bool onICMPMessage();
	
public:

	void ping(const IPAddress& targetIP);

	virtual void onPingReply(uint16_t roundtripTime) = 0;
};



#endif