
#include "Socket.h"
#include "inet.h"
#include "ethernet.h"
#include "Socket.h"
#include <avr/pgmspace.h>

MACAddress mymac PROGMEM = { 0x02, 0x21 ,0xcc ,0x4a ,0x79, 0x79 };
IPAddress testIP = { 192, 168, 1, 89 };
IPAddress myIP PROGMEM = { 192, 168, 1, 222 };



class MyProtocol : public Socket
{

public:
	
	void onConnect()
	{
		Serial.println("on connect");

		char * req = "GET / HTTP/1.1\r\nAccept:*/*\r\n\r\n";

		write(strlen(req), (byte*) req);

	}

	void onClose()
	{
		Serial.println("on close");
		close();
	}

	void onReceive(uint16_t len, const byte* data)
	{
		
		Serial.print("onReceive: "); Serial.print(len); Serial.println(" bytes");
	}


} socket;




unsigned long waitTimer = 0;
void setup()
{	



	Serial.begin(115200);

	Serial.println("Press any key to start...");

	while (!Serial.available());






	eth::localIP.set_P(&myIP);
	eth::localMAC.set_P( &mymac);

	if (!eth::begin(10))
		Serial.println("failed to start ethersocket");

	Serial.println("waiting for link...");

	while (!eth::isLinkUp());

	Serial.println("link is up");

	socket.remoteAddress = testIP;
	socket.remotePort.setValue(80);

	//Serial.print("resolving IP...");
	//while (!eth::whoHas(testIP))
	//	EtherSocket::packetReceiveChunk();

	//Serial.println("resolved.");

	


	socket.connect();
		
	waitTimer = millis()+1000;
}



void loop()
{
	EtherSocket::loop();

	if ((long)(millis() - waitTimer) >= 0)
	{
		//Serial.println((int) eth::whoHas(testIP));
		Serial.print("alive"); Serial.println(millis());


		waitTimer = millis() + 1000;
	}


}
