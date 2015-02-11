#include <ACross.h>
#include <Checksum.h>
#include <TCPSocket.h>
#include <UDPSocket.h>

#include <inet.h>
#include <EtherFlow.h>
#include <DNS.h>
#include <FlowScanner.h>


MACAddress_P mymac = { 0x02, 0x21 ,0xcc ,0x4a ,0x79, 0x79 };
IPAddress testIP = { 192, 168, 1, 88 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP  = { 192, 168, 1, 222 };
IPAddress_P netmask  = { 255, 255, 255, 0 };
//IPAddress_P dns = { 8, 8, 8, 8 };



class MyProtocol : public TCPSocket
{

public:
	
	void onConnect()
	{
		Serial.println("on connect");

		char * req = "GET / HTTP/1.1\r\nAccept:*" "/" "*\r\n\r\n";

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


} sck;


class MyUDP : public UDPSocket
{

public:
	void onReceive(uint16_t len, uint16_t datagramLength, const byte* data)
	{
		Serial.print("on UDP receive");
	}


}udp;



unsigned long waitTimer = 0;
void setup()
{	



	Serial.begin(115200);

	Serial.println("Press any key to start...");

	while (!Serial.available());




	net::localIP = myIP;
	net::localMAC = mymac;
	net::gatewayIP = gatewayIP;
	net::netmask = netmask;


	if (!net::begin(10))
		Serial.println("failed to start EtherFlow");

	Serial.println("waiting for link...");

	while (!net::isLinkUp());

	Serial.println("link is up");

	sck.remoteIP = testIP;
	sck.remotePort.setValue(80);

	//Serial.print("resolving IP...");
	//while (!eth::whoHas(testIP))
	//	EtherFlow::packetReceiveChunk();

	//Serial.println("resolved.");

	net::DNS.serverIP() = IPADDR_P(8, 8, 8, 8);
	

	net::DNS.resolve("www.peletier.com");

	sck.connect();
	
	
	//udp.localPort.setValue(1111);
	//udp.remotePort.setValue(53);
	//udp.remoteIP = testIP;

	//udp.write(strlen("HELLO WORLD"),(uint8_t*) "HELLO WORLD");

	//udp.send();

		
	waitTimer = millis()+1000;
}



void loop()
{
	NetworkService::loop();

	if ((long)(millis() - waitTimer) >= 0)
	{
		//Serial.println((int) eth::whoHas(testIP));
		Serial.print("alive"); Serial.println(millis());

		if (net::DNS.resolve("www.friendev.com"))
		{
			Serial.print("resolved. IP="); Serial.println(net::DNS.resolvedIP.b[0]);
		}

		waitTimer = millis() + 1000;
	}


}
