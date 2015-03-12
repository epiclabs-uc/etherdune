#ifndef _ICMPAUTOREPLY_H_
#define _ICMPAUTOREPLY_H_

#include <ACross.h>
#include "ICMP.h"

class ICMPPingAutoReply : protected ICMP
{

private:

	bool onICMPMessage();

};


#endif
