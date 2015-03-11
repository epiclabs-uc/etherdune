#include <ACross.h>
#include <FlowScanner.h>
#include <DNS.h>


#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("DNSDemo");


MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP = { 192, 168, 1, 33 };
IPAddress_P netmask = { 255, 255, 255, 0 };
IPAddress_P dnsIP = { 8, 8, 8, 8 };

DNSClient net::DNS;


class DNSResolver : public NetworkService
{
	uint16_t id;

	void onDNSResolve(uint8_t status, uint16_t identification, const IPAddress& ip)
	{
		if (identification == id)
		{
			if (status == 0)
			{
				id = 0;
				Serial.print(F("Resolved hostname to: "));
				//Serial.println(ip.toString());

			}
			else
			{
				Serial.println("Could not resolve hostname");
			}
		}
	}

public:
	void resolve(const char* hostName)
	{
		id = net::DNS.resolve(hostName);
		Serial.print(F("Resolving "));
		Serial.print(hostName);
		Serial.println(F(" ..."));

	}

} resolver;



void setup()
{

	Serial.begin(115200);
	ACross::init();

	Serial.println(F("EtherFlow DNS sample"));
	Serial.println(F("Press any key to start..."));

	while (!Serial.available());

	net::localMAC = mymac;
	net::localIP = myIP;
	net::gatewayIP = gatewayIP;
	net::netmask = netmask;
	net::dnsIP = dnsIP;

	if (!net::begin(10))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");


	resolver.resolve("www.friendev.com");

}


void loop()
{
	net::loop();

}

