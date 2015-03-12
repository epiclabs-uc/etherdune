#if !defined(ARDUINO)



#include <stdio.h>
#include <tchar.h>

#include <ACross.h>

#include <VirtualHardware\AConfig.h>
#include <VirtualHardware\ACRPCCLient.h>


// "EtherSocketTest.ino"
//#include "../EchoServer/EchoServer.ino"
//#include "../ChatServer/ChatServer.ino"
//#include "../TCPClientDemo_DaytimeClient/TCPClientDemo_DaytimeClient.ino"
//#include "../HTTPClientDemo_REST/HTTPClientDemo_REST.ino"
#include "../PingTest/PingTest.ino"
//#include "../DHCPTest/DHCPTest.ino"
//#include "../HTTPServerDemo/HTTPServerDemo.ino"
//#include "../DNSDemo/DNSDemo.ino"


int _tmain(int argc, _TCHAR* argv[])
{
	AConfig::VirtualSerialPort = _T("\\\\.\\COM31");
	AConfig::HostlinkSerialPort = _T("\\\\.\\COM3");

	ACRPCCLient::begin();



	setup();

	while (1)
		loop();

	return 0;
}

#endif