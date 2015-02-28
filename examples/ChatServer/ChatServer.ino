// Echo Server sample
// Listens for TCP connections on port 2500,
// Launches 1 socket per incoming connection, thus able to serve multiple clients simultaneously
// once connected,sends a welcome message, then the information received is relayed to all clients


#include <ACross.h>
#include <TCPSocket.h>
#include <FlowScanner.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("ChatServer");


MACAddress_P mymac = { 0x02, 0x21 ,0xee ,0x4a ,0x79, 0x79 };
IPAddress_P gatewayIP = { 192, 168, 1, 1 };
IPAddress_P myIP  = { 192, 168, 1, 33 };
IPAddress_P netmask  = { 255, 255, 255, 0 };

static const uint8_t CS_PIN = 10;



static const uint8_t MAX_CLIENTS = 4;
static const uint16_t CHAT_SERVER_TCP_PORT = 2500;

class ChatServer : public TCPSocket
{
private:
	
	static ChatServer clients[MAX_CLIENTS];

public:
	
	void start(uint16_t port)
	{
		localPort.setValue(port);
		listen();
	}

	void onConnect()
	{
		ACTRACE("onConnect");
		Serial.println(F("New client connected"));
		say(F(">> Somebody joined the room\n"));

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			ChatServer& sck = clients[i];
			if (sck.state == SCK_STATE_CLOSED)
			{
				sck.accept(*this);
				sck.write(F("Welcome to the chat room\n")); //accept connection and send a welcome message
				return;
			}
		}

		ACERROR("No more connections available"); //ignore request

	}

	void onClose()
	{
		close(); //properly close the connection.
		Serial.println(F("Client disconnected."));
		say(F(">> Someone left the room\n"));
	}

	void say(uint16_t len, const byte* data, ChatServer* exclude)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			ChatServer& sck = clients[i];
			if (&sck != exclude && sck.state == SCK_STATE_ESTABLISHED)
			{
				sck.write(len, data);
			}
		}
	}

	void say(const String& st, ChatServer* exclude = NULL)
	{
		say(st.length(), (byte*)st.c_str(),exclude);
	}

	void onReceive(uint16_t len, const byte* data)
	{
		ACTRACE("onReceive: %d bytes",len);

		say(len, data, this);
	}


} chatServer;

ChatServer ChatServer::clients[MAX_CLIENTS];



void setup()
{	
	Serial.begin(115200);
	ACross::init();
	Serial.println(F("Etherflow ChatServer sample"));
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


	chatServer.start(CHAT_SERVER_TCP_PORT);
	Serial.println(F("Chat server is up"));
}



void loop()
{
	NetworkService::loop();
}