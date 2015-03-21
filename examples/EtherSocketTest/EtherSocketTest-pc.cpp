// EtherDune scratchpad ACross Cross-compile project
// Author: Javier Peletier <jm@friendev.com>
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
//#include "../PingTest/PingTest.ino"
//#include "../DHCPTest/DHCPTest.ino"
//#include "../HTTPServerDemo/HTTPServerDemo.ino"
//#include "../DNSDemo/DNSDemo.ino"
#include "../UDPClientDemo_NTPClient/UDPClientDemo_NTPClient.ino"

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