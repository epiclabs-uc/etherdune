#include <ACross.h>
#include <TCPSocket.h>
#include <FlowScanner.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("ServerSocketTest");


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
		ACTRACE("onConnect");

		accept();

		write("How about a nice game of chess?\n");

	}

	void onClose()
	{
		close();
		listen();
	}

	void onReceive(uint16_t len, const byte* data)
	{
		ACTRACE("onReceive: %d bytes",len);
		Serial.write(data, len);
	}


	void onDNSResolve(uint16_t identification, const IPAddress& ip)
	{
		ACTRACE("resolved. IP=%d.%d.%d.%d", ip.b[0], ip.b[1], ip.b[2], ip.b[3]);
	}


} sck;




unsigned long waitTimer = 0;

void setup()
{	
	
	ACross::init();
	Serial.begin(115200);
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

	sck.localPort.setValue(80);
	sck.listen();

	waitTimer = millis()+1000;
}



void loop()
{
	NetworkService::loop();

	if ((long)(millis() - waitTimer) >= 0)
	{

		waitTimer = millis() + 1000;
	}


}
