// This code slightly follows the conventions of, but is not derived from:
//      EHTERSHIELD_H library for Arduino etherShield
//      Copyright (c) 2008 Xing Yu.  All right reserved. (this is LGPL v2.1)
// It is however derived from the enc28j60 and ip code (which is GPL v2)
//      Author: Pascal Stang
//      Modified by: Guido Socher
//      DHCP code: Andrew Lindsay
// Hence: GPL V2
//
// 2010-05-19 <jc@wippler.nl>

#include <ethernet.h>
#include <stdarg.h>
#include <avr/eeprom.h>


EtherCard ether;

uint8_t EtherCard::mymac[6];  // my MAC address
uint8_t EtherCard::myip[4];   // my ip address
uint8_t EtherCard::netmask[4]; // subnet mask
uint8_t EtherCard::broadcastip[4]; // broadcast address
uint8_t EtherCard::gwip[4];   // gateway
uint8_t EtherCard::dhcpip[4]; // dhcp server
uint8_t EtherCard::dnsip[4];  // dns server
uint8_t EtherCard::hisip[4];  // ip address of remote host
uint16_t EtherCard::hisport = 80; // tcp port to browse to
bool EtherCard::using_dhcp = false;
bool EtherCard::persist_tcp_connection = false;
int16_t EtherCard::delaycnt = 0; //request gateway ARP lookup

uint8_t EtherCard::begin(const uint16_t size,
	const uint8_t* macaddr,
	uint8_t csPin) {
	using_dhcp = false;
	Stash::initMap(56);
	copyMac(mymac, macaddr);
	return initialize(size, mymac, csPin);
}

bool EtherCard::staticSetup(const uint8_t* my_ip,
	const uint8_t* gw_ip,
	const uint8_t* dns_ip,
	const uint8_t* mask) {
	using_dhcp = false;

	if (my_ip != 0)
		copyIp(myip, my_ip);
	if (gw_ip != 0)
		setGwIp(gw_ip);
	if (dns_ip != 0)
		copyIp(dnsip, dns_ip);
	if (mask != 0)
		copyIp(netmask, mask);
	updateBroadcastAddress();
	delaycnt = 0; //request gateway ARP lookup
	return true;
}
