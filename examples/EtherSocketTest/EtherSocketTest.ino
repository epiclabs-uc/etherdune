
#include "Socket.h"
#include "inet.h"
#include "ethernet.h"

MACAddress mymac = { 1, 2, 3, 4, 5, 6 };

void setup()
{

	if (!eth.begin(mymac, 10))
		Serial.println("failed to start ethersocket");

	

}

void loop()
{

  /* add main program code here */

}
