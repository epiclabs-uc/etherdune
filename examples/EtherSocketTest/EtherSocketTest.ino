#include <ACross.h>
#include <Checksum.h>
#include <TCPSocket.h>
#include <UDPSocket.h>

#include <inet.h>
#include <EtherFlow.h>
#include <DNS.h>
#include <FlowScanner.h>
#include <HTTPClient.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("EtherSocketTest");


MACAddress_P mymac = { 0x02, 0x21 ,0xee ,0x4a ,0x79, 0x79 };
IPAddress testIP = /*{ 192,168,4,1 };*/ { 85,214,129,67 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP  = { 192, 168, 1, 33 };
IPAddress_P netmask  = { 255, 255, 255, 0 };
//IPAddress_P dns = { 8, 8, 8, 8 };



class MyProtocol : public TCPSocket
{

public:
	
	void onConnect()
	{
		ACTRACE("on connect");

		char * req = "GET / HTTP/1.1\r\nAccept:*" "/" "*\r\n\r\n";

		write(strlen(req), (byte*) req);



	}

	void onClose()
	{
		ACTRACE("on close");
		close();
	}

	void onReceive(uint16_t len, const byte* data)
	{
		ACTRACE("onReceive: %d bytes",len);
	}


	void onDNSResolve(uint16_t identification, const IPAddress& ip)
	{
		ACTRACE("resolved. IP=%d.%d.%d.%d", ip.b[0], ip.b[1], ip.b[2], ip.b[3]);
	}


} sck;


class MyUDP : public UDPSocket
{

public:
	bool onReceive(uint16_t len, uint16_t datagramLength, const byte* data)
	{
		ACTRACE("on UDP receive");
		return true;
	}


}udp;


class MyHTTPClient : public HTTPClient
{

public:

	void start()
	{
		request(F("www.playersketch.com"), F("/"));
	}

	void onHeaderReceived(uint16_t len, const byte* data)
	{
		//dsprint("'"); dprint(header); dsprint("'='"); dprint(value); dsprintln("'");

	
		Serial.write(data, len);

	}

	void onResponseReceived() 
	{
		ACTRACE("HTTP status=%d",statusCode);

	}
	void onResponseEnd()
	{
		ACTRACE("HTTP session end");
	}
	void onBodyReceived(uint16_t len, const byte* data)
	{
		ACTRACE("HTTP bytes received=%d",len);
	}


}http;


unsigned long waitTimer = 0;

void setup()
{	
	Serial.begin(115200);
	ACross::init();
	
	Serial.println(F("Press any key to start..."));

	while (!Serial.available());


	net::localIP = myIP;
	net::localMAC = mymac;
	net::gatewayIP = gatewayIP;
	net::netmask = netmask;


	if (!net::begin(10))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");

	sck.remoteIP = testIP;
	sck.remotePort.setValue(80);

	//Serial.print("resolving IP...");
	//while (!eth::whoHas(testIP))
	//	EtherFlow::packetReceiveChunk();

	//Serial.println("resolved.");

	net::DNS.serverIP() = IPADDR_P(8, 8, 8, 8);
	
	net::DNS.resolve("www.friendev.com");



	//sck.connect();
	
	
	//udp.localPort.setValue(1111);
	//udp.remotePort.setValue(53);
	//udp.remoteIP = testIP;

	//udp.write(strlen("HELLO WORLD"),(uint8_t*) "HELLO WORLD");

	//udp.send();

		
	http.start();

	waitTimer = millis()+1000;
}



void loop()
{
	NetworkService::loop();

	if ((long)(millis() - waitTimer) >= 0)
	{
		//Serial.println((int) eth::whoHas(testIP));
		//Serial.print("alive"); Serial.println(millis());

		waitTimer = millis() + 1000;
	}


}
