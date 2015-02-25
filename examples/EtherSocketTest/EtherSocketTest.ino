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


MACAddress_P mymac = { 0x02, 0x21 ,0xee ,0x4a ,0x79, 0x79 };
IPAddress testIP = /*{ 192,168,4,1 };*/ { 85,214,129,67 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP  = { 192, 168, 1, 33 };
IPAddress_P netmask  = { 255, 255, 255, 0 };


class DMATest : public Socket
{
public:
	void start()
	{
		char* data = "this is some text";

		uint16_t swchecksum = Checksum::calc(strlen(data), (uint8_t*)data);

		ACINFO("Software Checksum=%d", swchecksum);

		EtherFlow::writeBuf(5000, strlen(data), (uint8_t*)data);
		nint16_t hwchecksum;
		hwchecksum.rawu = ~EtherFlow::hardwareChecksum(5000, strlen(data));

		ACINFO("Hardware Checksum=%d", hwchecksum.getValue());


		EtherFlow::writeBuf(RXSTART_INIT, strlen(data),(byte*)data);

		char rec[40];

		EtherFlow::readBuf(RXSTOP_INIT - 1, sizeof(rec), (byte*)rec);

		Serial.write((uint8_t*)rec, sizeof(rec));

	}

}dmatest;




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


	dmatest.start();

}


void loop()
{
	NetworkService::loop();

}
