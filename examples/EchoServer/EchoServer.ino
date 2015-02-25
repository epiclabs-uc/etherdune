// Echo Server sample
// Listens for TCP connections on port 80, sends a welcome message, then echoes back anything received


#include <ACross.h>
#include <TCPSocket.h>
#include <FlowScanner.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("EchoServer");


MACAddress_P mymac = { 0x02, 0x21 ,0xee ,0x4a ,0x79, 0x79 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP  = { 192, 168, 1, 33 };
IPAddress_P netmask  = { 255, 255, 255, 0 };

static const uint8_t CS_PIN = 10;

class EchoServer : public TCPSocket
{

public:
	
	void onConnect()
	{
		ACTRACE("onConnect");

		accept(); //accept connection and send a welcome message

		write(F("How about a nice game of chess?\n"));

	}

	void onClose()
	{
		close(); //property close the connection and then listen again.
		listen();
	}

	void onReceive(uint16_t len, const byte* data)
	{
		ACTRACE("onReceive: %d bytes",len);
		write(len, data); //echo everything back
	}


} sck;

void setup()
{	
	
	ACross::init();
	Serial.begin(115200);
	Serial.println(F("Etherflow Echo TCP server sample"));
	Serial.print(F("Free RAM: ")); Serial.println(ACross::getFreeRam());
	Serial.println(F("Press any key to start..."));

	while (!Serial.available());


	net::localIP = myIP;
	net::localMAC = mymac;
	net::gatewayIP = gatewayIP;
	net::netmask = netmask;


	if (!net::begin(CS_PIN))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");

	sck.localPort.setValue(80);
	sck.listen();

}



void loop()
{
	NetworkService::loop();
}
