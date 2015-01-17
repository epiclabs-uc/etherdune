
#include "Socket.h"
#include "inet.h"
#include "ethernet.h"
#include <avr/pgmspace.h>

MACAddress mymac PROGMEM = { 1,2,3,4,5,6 };
IPAddress testIP = { 192, 168, 1, 1 };
IPAddress myIP PROGMEM = { 192, 168, 1, 200 };

void setup()
{
	Serial.begin(115200);

	eth::localIP.set_P(&myIP);
	eth::localMAC.set_P( &mymac);

	if (!eth::begin(10))
		Serial.println("failed to start ethersocket");

	Serial.println("waiting for link...");

	//while (!eth::isLinkUp());

	//Serial.println("link is up");



}

unsigned long waitTimer = 0;

void loop()
{

	EtherSocket::loop();

	if ((long)(millis() - waitTimer) >= 0)
	{
		Serial.println((int) eth::whoHas(testIP));
		waitTimer += 1000;
	}


}
