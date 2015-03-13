#include <ACross.h>
#include <Checksum.h>
#include <TCPSocket.h>
#include <UDPSocket.h>

#include <inet.h>
#include <ENC28J60.h>
#include <DNS.h>
#include <FlowScanner.h>
#include <HTTPClient.h>
#include <DHCP.h>


#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("EtherSocketTest");


static const uint8_t CS_PIN = 10; //Put here what pin you are using for your ENC28J60's chip select
static MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };
static IPAddress_P gatewayIP = { 192, 168, 1, 1 };
static IPAddress_P myIP = { 192, 168, 1, 33 };
static IPAddress_P netmask = { 255, 255, 255, 0 };

DHCP dhcp;


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

	if (!net::begin(CS_PIN))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");

	

	//if (!dhcp.dhcpSetup())
	//{
	//	Serial.println(F("DHCP setup failed"));
	//	ACross::halt(1);
	//}

	Serial.println(F("DHCP setup OK"));
	

}


void loop()
{
	net::loop();

}

