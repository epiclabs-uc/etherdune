//Demonstrates the HTTPClient class by connecting to a weather REST service and extracting
// the local temperature of a city.

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
ACROSS_MODULE("HTTPClient demo");


MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP = { 192, 168, 1, 33 };
IPAddress_P netmask = { 255, 255, 255, 0 };
IPAddress_P dns = { 8, 8, 8, 8 };

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
		//Serial.write(data, len);

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


	net::localIP = myIP;
	net::localMAC = mymac;
	net::gatewayIP = gatewayIP;
	net::netmask = netmask;
	net::DNS.serverIP() = dns;


	if (!net::begin(10))
	{
		ACERROR("failed to start EtherFlow");
		Serial.println(F("failed to start EtherFlow"));	
	}

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");

	

	http.start();

	Serial.println("Connecting...");

}



void loop()
{
	NetworkService::loop();

}
