#include <ACross.h>
#include <FlowScanner.h>
#include <DHCP.h>


#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("DHCPTest");


static const uint8_t CS_PIN = 10; //Put here what pin you are using for your ENC28J60's chip select
static MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };

DHCP dhcp;


void setup()
{

	Serial.begin(115200);
	ACross::init();

	Serial.println(F("EtherFlow DHCP sample"));
	Serial.println(F("Press any key to start..."));

	while (!Serial.available());

	net::localMAC = mymac;

	if (!net::begin(CS_PIN))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");

	Serial.println("Obtaining DHCP configuration...");

	if (!dhcp.dhcpSetup())
	{
		Serial.println(F("DHCP setup failed"));
		ACross::halt(1);
	}

	Serial.println(F("DHCP setup OK"));

	Serial.println(F("DHCP config:"));
	Serial.print(F("Local IP: "));
	Serial.println(net::localIP.toString());
	Serial.print(F("Network mask: "));
	Serial.println(net::netmask.toString());
	Serial.print(F("Gateway IP: "));
	Serial.println(net::gatewayIP.toString());
	Serial.print(F("DNS IP: "));
	Serial.println(net::dnsIP.toString());

	
}


void loop()
{
	net::loop();

}

