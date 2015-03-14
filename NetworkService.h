// EtherFlow Network Service base class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Base class for any network service running in EtherFlow 
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


/// \class NetworkService
/// \brief Abstract base class for any network service running in EtherFlow
/// \details The NetworkService class is the core of the EtherFlow library. 
/// This class includes methods and virtual functions (events) that provide 
/// access to the network layer, providing an extensible framework for building reusable 
/// and maintenable components and maximizing code reuse.
///
/// To extend the functionality of EtherFlow to new protocols, create a new class 
/// derived from NetworkService. 
///
/// How does this work ?
///
/// All instances of NetworkService-derived classes get low-level Ethernet access
/// in this way: Every time an Ethernet packet is received, NetworkService calls each 
/// instance's onPacketReceived() event handler. The receiving instance then has the 
/// chance to look at the packet and determine whether or not the packet belongs to it. 
/// If it decides it owns the packet, then it will return `true` to claim it and process it.
/// If does not own the packet, it will return false and NetworkService will query the next 
/// instance. If no instance claims the packet, the packet is discarded.
///
/// For example, an open UDP socket listening on a given port (implemented in EtherFlow 
/// as the UDPSocket class) will check that the Ethernet Type field in the ethernet header
/// is set to the IP protocol (`packet.eth.etherType =` \ref ETHTYPE_IP), then check whether 
/// the IP Header indicates `UDP` as protocol type(`packet.ip.protocol =` \ref IP_PROTO_UDP) 
/// and then finally check if the port its listening on matches the destination port UDP 
/// local port (`packet.udp.destinationPort`) for example:
///
/// \code
///
/// bool UDPSocket::onPacketReceived()
/// {
///		if (
///			packet.eth.etherType == ETHTYPE_IP &&
///			packet.ip.protocol == IP_PROTO_UDP &&
///			localPort == packet.udp.destinationPort)
///		{
///			// This packet is targeted at us. Process packet
///
///			//
///			// [packet processing code]
///			//
///
///			return true; // this instance claimed this packet and processed it
///		}
///		else
///		{
///			return false; // this packet is not mine, ask someone else.
///		}
/// }
///
/// \endcode
///
/// EtherFlow comes with the following protocols out of the box, implemented as 
/// NetworkService - derived classes :
///
/// - ARP (ARPService)
/// - %ICMP Echo request / reply (ICMPPinger and ICMPPingAutoReply)
/// - TCPSocket
/// - UDPSocket
/// - DNS (DNSClient, derived from UDPSocket)
/// - DHCP (derived fom UDPSocket)
/// - HTTPClient (derived from TCPSocket)
/// - HTTPServer (derived from TCPSocket)
///
/// Out of this list, \ref ARP is instantiated by default to handle %ARP traffic and respond to
/// %ARP queries from other hosts. Other basic services, like %DHCP, %DNS or ICMP Echo must 
/// be explicitly instantiated by the developer if needed in the application. If a service 
/// is not used, then it is not compiled in and therefore does not waste RAM or program space.
///
/// To create a new network service or protocol, create a new class derived from the most 
/// appropriate descendant of NetworkService or NetworkService itself. Most of the time this
/// will be the TCPSocket and UDPSocket classes.

#ifndef __NETWORKSERVICE_H_
#define __NETWORKSERVICE_H_

#include "ENC28J60.h"
#include "List.h"

class ARPService;
class DNSClient;


class NetworkService : private ListItem, public ENC28J60
{
	friend class DNSClient;
	friend class ENC28J60;

private:

	static List activeServices;

	static void processIncomingPacket();
	static void notifyOnDNSResolve(uint8_t status, uint16_t identification, const IPAddress& ip);
	
protected:

	static EthBuffer packet; //!< in-memory packet buffer currently being processed. See EthBuffer for more information.
	static ARPService ARP; //!< ARP singleton instance. Maintains ARP table and provides methods to query hosts in the network. See ARPService for more information.

	NetworkService();
	~NetworkService();

	static bool sendIPPacket(uint16_t length); 
	static void prepareIPPacket(const IPAddress& remoteIP);
	static bool sameLAN(IPAddress& dst);

	/// <summary>
	/// Called every time a packet is received. This is a pure virtual function -- must be overriden in all derived classes.
	/// </summary>
	/// <returns>The derived instance must return `true` to claim the packet or `false` to ignore it.</returns>
	virtual bool onPacketReceived() = 0;
	
	
	virtual void tick();
	virtual void onDNSResolve(uint8_t status, uint16_t identification, const IPAddress& ip);

public:
	static MACAddress localMAC; //!< Ethernet MAC address. Set this to your desired MAC address before you call begin()
	static IPAddress localIP; //!< IP address of this application
	static IPAddress gatewayIP; //!< IP address of the gateway in this network
	static IPAddress netmask; //!< Subnet mask
	static IPAddress dnsIP; //!< IP address of the DNS server to use.
	

	static bool begin(uint8_t cspin);
	static void loop();
	static DNSClient& DNS();

};

typedef NetworkService net;
 

#endif