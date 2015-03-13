// Echo Server sample
// Listens for TCP connections on port 2500, sends a welcome message, then echoes back anything received


#include <ACross.h>
#include <TCPSocket.h>
#include <FlowScanner.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("EchoServer");


static const uint8_t CS_PIN = 10; //Put here what pin you are using for your ENC28J60's chip select
MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP  = { 192, 168, 1, 33 };
IPAddress_P netmask  = { 255, 255, 255, 0 };

static const uint8_t CS_PIN = 10;
static const uint16_t ECHO_SERVER_TCP_PORT = 2500;

class EchoServer : public TCPSocket
{

public:
	
	void start(uint16_t port)
	{
		localPort.setValue(port);
		listen();
	}

	void onConnectRequest()
	{
		ACTRACE("onConnectRequest");
		accept(); //accept connection.
	}
	void onConnect()
	{
		ACTRACE("onConnect");

		write(F("How about a nice game of chess?\n"));
		Serial.println(F("Client connected"));

	}

	void onClose()
	{
		close(); //property close the connection and then listen again.
		Serial.println(F("Client disconnected"));
	}

	void onReceive(uint16_t len, const byte* data)
	{
		ACTRACE("onReceive: %d bytes",len);
		write(len, data); //echo everything back
	}

	void onTerminate()
	{
		listen();
	}


} echoServer;

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

	echoServer.start(ECHO_SERVER_TCP_PORT);

	Serial.println(F("Echo server is up"));
}



void loop()
{
	net::loop();
}
