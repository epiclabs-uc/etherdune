//Demonstrates the TCP Client class by connecting to a DAYTIME server (TCP port 13) and retrieving the current time



#include <ACross.h>
#include <FlowScanner.h>
#include <Checksum.h>
#include <TCPSocket.h>
#include <UDPSocket.h>

#include <inet.h>

#include <DNS.h>

#define AC_LOGLEVEL 5
#include <ACLog.h>
ACROSS_MODULE("TCPClient Demo");


MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP = { 192, 168, 1, 33 };
IPAddress_P netmask = { 255, 255, 255, 0 };

DNSClient net::DNS;

class MyProtocol : public TCPSocket
{
	uint16_t DNSid;
public:

	void start()
	{
		remotePort.setValue(13);

		DNSid = net::DNS.resolve("ntp-nist.ldsbc.edu");
		//alternatives here: http://www.jumpjet.info/Offbeat-Internet/Public/Daytime/url.htm
	}

	void onConnect()
	{
		ACTRACE("on connect");
		Serial.print(F("Time is:"));
	}

	void onClose()
	{
		ACTRACE("on close");
		close(); //properly close the connection.
	}

	void onReceive(uint16_t len, const byte* data)
	{
		ACTRACE("onReceive: %d bytes", len);
		Serial.write(data, len);
		//time info comes in this format
		//http://www.nist.gov/pml/div688/grp40/its.cfm
		//this code can be improved by parsing the received string to sync Arduino's own timer, for example
	}


	void onDNSResolve(uint8_t status, uint16_t identification, const IPAddress& resolvedIP)
	{
		
		if (identification == DNSid && status==0)
		{
			ACINFO("resolved. IP=%d.%d.%d.%d", resolvedIP.b[0], resolvedIP.b[1], resolvedIP.b[2], resolvedIP.b[3]);
			remoteIP = resolvedIP;
			connect();
		}
	}


} sck;


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
	net::dnsIP = IPADDR_P(8, 8, 8, 8);

	if (!net::begin(10))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");

	

	Serial.println(F("connecting..."));
	sck.start();

}



void loop()
{
	net::loop();


}
