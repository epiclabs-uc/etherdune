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


MACAddress_P mymac = { 0x02, 0x21, 0xee, 0x4a, 0x79, 0x79 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP = { 192, 168, 1, 33 };
IPAddress_P netmask = { 255, 255, 255, 0 };


DEFINE_FLOWPATTERN(temperaturePattern, "\"temp\":%d%d");

class MyHTTPClient : public HTTPClient
{

public:

	float temp;


	void start()
	{
		request(F("api.openweathermap.org"), F("/data/2.5/weather?q=Madrid,es&units=metric"));
	}

	void onHeaderReceived(uint16_t len, const byte* data)
	{

		Serial.write(data, len);

	}

	void onResponseReceived()
	{
		ACTRACE("HTTP status=%d", statusCode);

	}
	void onBodyReceived(uint16_t len, const byte* data)
	{
		ACTRACE("HTTP bytes received=%d", len);
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

	net::DNS.serverIP() = IPADDR_P(8, 8, 8, 8);


	http.start();

	waitTimer = millis() + 1000;
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
