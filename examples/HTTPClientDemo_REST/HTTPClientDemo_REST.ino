//Demonstrates the HTTPClient class by connecting to a weather REST service and extracting
// the local temperature of a city.

#include <ACross.h>
#include <FlowScanner.h>
#include <HTTPClient.h>
#include <DNS.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("HTTPClient demo");

#define DHCP_ENABLE true

static MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };

//the following parameters will be ignored and not linked if DHCP_ENABLE is set to true.
static IPAddress_P gatewayIP = { 192, 168, 1, 1 };
static IPAddress_P myIP = { 192, 168, 1, 33 };
static IPAddress_P netmask = { 255, 255, 255, 0 };
static IPAddress_P dns = { 8, 8, 8, 8 };


#if DHCP_ENABLE

#include <DHCP.h>
DHCP dhcp;

#else

#endif


DNSClient net::DNS;

DEFINE_FLOWPATTERN(temperaturePattern, "\"temp\":%7[^,]"); // look for a string like this "temp":12.321, and extract the values

class MyHTTPClient : public HTTPClient
{

public:

	char temp[8]; //temporary buffer to hold the captured temperature string

	void start()
	{
		request(F("api.openweathermap.org"), F("/data/2.5/weather?q=Madrid,es&units=metric"));
		//see http://openweathermap.org/current
	}

	void onHeaderReceived(uint16_t len, const byte* data)
	{
		//Serial.write(data, len);
	}

	void onResponseReceived()
	{
		ACTRACE("HTTP status=%d", statusCode);

	}

	void onBodyBegin()
	{
		scanner.setPattern(temperaturePattern);
	}

	void onBodyReceived(uint16_t len, const byte* data)
	{
		ACTRACE("HTTP bytes received=%d", len);

		byte* buf = (byte*)data;
		
		if (temperaturePattern.signaled)
			return;

		if (scanner.scan(&buf, &len, temp))
		{
			float t = atof(temp);

			Serial.print(F("The temperature is "));
			Serial.print(t);
			Serial.println('C');
			close();
			return;
		}

	}
	void onResponseEnd()
	{
		ACTRACE("HTTP session end");
	}

}http;


unsigned long waitTimer = 0;

void setup()
{
	Serial.begin(115200);
	ACross::init();
	Serial.println(F("Etherflow HTTPClient sample"));
	Serial.print(F("Free RAM: ")); Serial.println(ACross::getFreeRam());

	Serial.println(F("Press any key to start..."));

	while (!Serial.available());

	net::localMAC = mymac;

	if (!net::begin(10))
	{
		ACERROR("failed to start EtherFlow");
		Serial.println(F("failed to start EtherFlow"));
	}

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");


#if DHCP_ENABLE
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
#else
	net::localIP = myIP;
	net::gatewayIP = gatewayIP;
	net::netmask = netmask;
	net::dnsIP = dns;

#endif

	http.start();

	Serial.println("Connecting...");

}



void loop()
{
	net::loop();

}
