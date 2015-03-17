// EtherFlow Chat Server demo
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements a simple chat server, demonstrating TCP and how to handle
// multiple simutaneous clients
//
// Copyright (c) 2015 All Rights Reserved, http://friendev.com
//
// This source is subject to the GPLv2 license.
// Please see the License.txt file for more information.
// All other rights reserved.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.

/**
\file 
\brief Implements a simple chat server, demonstrating TCP and how to 
handle multiple simutaneous clients
\details
Listens for TCP connections on port 2500, then launches 1 socket per 
incoming connection, thus able to serve multiple clients simultaneously.
When a client connects, the server sends a welcome message, then any message
received is relayed to all clients.

See the TCPSocket documentation for more information.

\cond


*/

#include <ACross.h>
#include <TCPSocket.h>
#include <FlowScanner.h>

#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("ChatServer");


static const uint8_t CS_PIN = 10; //Put here what pin you are using for your ENC28J60's chip select
static MACAddress_P mymac = { 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64 };
static IPAddress_P gatewayIP = { 192, 168, 1, 1 };
static IPAddress_P myIP  = { 192, 168, 1, 33 };
static IPAddress_P netmask  = { 255, 255, 255, 0 };

static const uint8_t MAX_CLIENTS = 4;
static const uint16_t CHAT_SERVER_TCP_PORT = 2500;

class ChatServer : public TCPSocket
{
private:
	
	static ChatServer clients[MAX_CLIENTS];

public:
	
	void start(uint16_t port)
	{
		localPort =port;
		listen();
	}

	void onConnectRequest()
	{
		ACTRACE("onConnectRequest");

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			ChatServer& sck = clients[i];
			if (sck.getState() == SCK_STATE_CLOSED)
			{
				sck.accept(*this); // accept connection
				return;
			}
		}

		ACERROR("No more connections available"); //ignore request

	}

	void onConnect()
	{
		write(F("Welcome to the chat room\n")); //send a welcome message
		Serial.println(F("New client connected"));
		say(F(">> Somebody joined the room\n"),this);
	}

	void onClose()
	{
		close(); //properly close the connection.
	}

	void onTerminate()
	{
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
	net::loop();
}


/// \endcond