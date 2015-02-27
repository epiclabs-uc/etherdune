#if !defined(ARDUINO)



#include <stdio.h>
#include <tchar.h>

#include <ACross.h>

#include <VirtualHardware\AConfig.h>
#include <VirtualHardware\ACRPCCLient.h>


//#include "EtherSocketTest.ino"
//#include "../EchoServer/EchoServer.ino"
//#include "../ChatServer/ChatServer.ino"
#include "../TCPClientDemo_DaytimeClient/TCPClientDemo_DaytimeClient.ino"

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