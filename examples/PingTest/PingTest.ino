#include <ACross.h>
#include <Checksum.h>
#include <TCPSocket.h>
#include <UDPSocket.h>

#include <inet.h>
#include <ENC28J60.h>
#include <DNS.h>
#include <FlowScanner.h>
#include <HTTPClient.h>
#include <ICMPPingAutoReply.h>
#include <ICMPPinger.h>
#include <DHCP.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("PingTest");


MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };

DHCP dhcp;

// Instantiate ICMPPingAutoReply class, takes care of replying to incoming echo requests
ICMPPingAutoReply pingAutoReply; 

class ICMPHandler : ICMPPinger
{
	static const uint8_t PING_PERIOD = NTICKS(1000); //ms
	uint8_t timer;
	IPAddress target;

	void onPingReply(uint16_t time)
	{
		printf_P(PSTR("Reply from %d.%d.%d.%d: bytes=%d time=%dms TTL=%d\n"),
			packet.ip.sourceIP.b[0], packet.ip.sourceIP.b[1], packet.ip.sourceIP.b[2], packet.ip.sourceIP.b[3],
			packet.ip.totalLength.getValue() - sizeof(IPHeader) - sizeof(ICMPHeader),
			time, packet.ip.TTL);

	}

	void tick()
	{
		timer--;

		if (timer == 0)
		{
			timer = PING_PERIOD;
			ping(target);
		}


		ICMPPinger::tick();
	}

public:
	void start(const IPAddress& targetIP)
	{
		timer = 1;
		target = targetIP;
		tick();
	}


}pingTest;


void setup()
{

	Serial.begin(115200);
	ACross::init();
#ifdef ACROSS_ARDUINO
	ACross::printf_serial_init();
#endif

	printf(PSTR("ICMP Ping EtherFlow sample\n"));
	printf(PSTR("Press any key to start...\n"));

	while (!Serial.available());
	
	net::localMAC = mymac;

	if (!net::begin(10))
		ACERROR("failed to start EtherFlow");

	ACINFO("waiting for link...");

	while (!net::isLinkUp());

	ACINFO("link is up");


	if (!dhcp.dhcpSetup())
	{
		Serial.println(F("DHCP setup failed"));
		ACross::halt(1);
	}

	Serial.println(F("DHCP setup OK"));

	IPAddress targetIP;
	targetIP = IPADDR_P(8, 8, 8, 8);
	pingTest.start(targetIP);

}

void loop()
{
	net::loop();

}

