// EtherDune Network Service base class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Base data structures for Internet communication
// Credits: Some help text taken from Wikipedia and the TCP/IP guide http://http://www.tcpipguide.com/
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
\brief Base data structures for Internet communication
\details This header file contains all the data structures to interpret data coming in the wire

*/

#ifndef __inet_h_
#define __inet_h_

#include <ACross.h>
#include "config.h"

union u16_t
{
	struct
	{
		uint8_t l;
		uint8_t h;
	};
	uint16_t v;
};
union u32_t
{
	struct
	{
		u16_t l;
		u16_t h;
	};
	uint32_t v;
};

/// <summary>Represents a network byte order 16 bit integer.
/// Includes overloaded operators so as to make conversion to/from 
/// network byte order transparent
///
/// </summary>
union nint16_t
{
	uint16_t rawValue; //!< provides low level access to the memory containing the network-order integer
	struct
	{
		uint8_t h; //!< most significant byte
		uint8_t l; //!< least significant byte
	};

	/// <summary>
	/// Converts the underlying network-order integer to little endian.
	/// </summary>
	/// <returns>
	/// uint16_t equivalent of the network-order integer
	/// </returns>
	operator uint16_t()
	{
		u16_t u;
		u.l = l;
		u.h = h;

		return u.v;
	}

	/// <summary>
	/// Assigns a little endian value to this network-order integer
	/// </summary>
	/// <param name="v">value to assign</param>
	void operator=(uint16_t v)
	{

		l = (uint8_t)v;
		h = v >> 8;
	}

	/// <summary>
	/// sets the variable to <c>0</c>.
	/// </summary>
	inline void zero()
	{
		rawValue = 0;
	}
};


/// <summary>Represents a network byte order 32 bit integer.
/// Includes overloaded operators so as to make conversion to/from 
/// network byte order transparent
///
/// </summary>
union nint32_t
{
	uint32_t rawValue; //!< provides low level access to the memory containing the network-order integer
	struct
	{
		nint16_t h; //!< most significant integer
		nint16_t l; //!< least significant integer
	};
	uint8_t raw[4];

	/// <summary>
	/// Converts the underlying network-order integer to little endian.
	/// </summary>
	/// <returns>
	/// uint32_t equivalent of the network-order integer
	/// </returns>
	operator uint32_t()
	{
		u32_t u;
		u.h.h = h.h;
		u.h.l = h.l;
		u.l.h = l.h;
		u.l.l = l.l;
		return u.v;
	}

	/// <summary>
	/// Assigns a little endian value to this network-order integer
	/// </summary>
	/// <param name="v">value to assign</param>
	void operator=(uint32_t v)
	{
		u32_t u;
		u.v = v;

		h.h = u.h.h;
		h.l = u.h.l;
		l.h = u.l.h;
		l.l = u.l.l;



	}

	/// <summary>
	/// sets the variable to <c>0</c>.
	/// </summary>
	inline void zero()
	{
		rawValue = 0;
	}

};

/// helper struct to store an IP address in PROGMEM
struct IPAddress_P_
{
	uint8_t b[4];
};

/// helper macro to store an IP address in PROGMEM
#define IPAddress_P PROGMEM IPAddress_P_ 

#ifdef ACROSS_ARDUINO
/// defines an IP address as stored in PROGMEM
#define IPADDR_P(b0,b1,b2,b3) (__extension__({static IPAddress_P_ __c PROGMEM = {(b0),(b1),(b2),(b3)}; &__c;}))
#else
/// defines an IP address as stored in PROGMEM
#define IPADDR_P(b0,b1,b2,b3) {{ (b0),(b1),(b2),(b3) }}
#endif

/// represents an IP address in memory
union IPAddress
{
	uint8_t b[4]; //!< byte-wise access to IP address
	uint32_t u; //!< access the IP as a uint32_t for convenience

	/// <summary>
	/// Assigns from an IP address stored in PROGMEM
	/// </summary>
	/// <param name="ip">The ip.</param>
	void operator = (IPAddress_P_* ip)
	{
		memcpy_P(b, ip, sizeof(IPAddress));
	}

	/// <summary>
	/// Assigns from an IP address stored in memory
	/// </summary>
	/// <param name="ip">The ip.</param>
	void operator = (IPAddress_P_& ip)
	{
		operator=(&ip);
	}

	/// <summary>
	/// Sets the IP address to zero.
	/// </summary>
	void zero()
	{
		u = 0;
	}

	/// <summary>
	/// Sets the IP to <c>255.255.255.255</c>
	/// </summary>
	void setBroadcastIP()
	{
		//u = 0xFFFFFFFF;
		*this = IPADDR_P(255, 255, 255, 255); // consumes 2 bytes less than the above, oh well...
	}

	/// <summary>
	/// Converts the IP address to a string for display.
	/// </summary>
	/// <returns>string representing the IP address</returns>
	/// <remarks>**Warning** if this function is used anywhere in your code, it will permanently allocate 16 bytes of RAM.
	/// It is recommended you only use it for debugging purposes.</remarks>
	char* toString() const
	{
		static char addressString[16];
		sprintf(addressString, "%d.%d.%d.%d", b[0], b[1], b[2], b[3]);
		return addressString;
	}

};

/// helper struct to store a MAC address in PROGMEM
struct MACAddress_P_
{
	uint8_t b[6];
};

#define MACAddress_P PROGMEM MACAddress_P_ 
#ifdef ACROSS_ARDUINO
/// stores a MAC address in PROGMEM
#define MACADDR_P(b0,b1,b2,b3,b4,b5) (__extension__({static MACAddress_P_ __c PROGMEM = {(b0),(b1),(b2),(b3),(b4),(b5)}; &__c;}))
#else
/// stores a MAC address in PROGMEM
#define MACADDR_P(b0,b1,b2,b3,b4,b5) {{ (b0),(b1),(b2),(b3),(b4),(b5) }}
#endif

struct MACAddress
{
	uint8_t b[6]; //!< byte-wise access to the MAC address

	/// <summary>
	/// Assigns from a MAC address stored in PROGMEM
	/// </summary>
	/// <param name="mac">The mac.</param>
	void operator = (MACAddress_P_* mac)
	{
		memcpy_P(b, mac, sizeof(MACAddress));
	}

	/// <summary>
	/// Assigns from a MAC address stored in RAM
	/// </summary>
	/// <param name="mac">The mac.</param>
	void operator = (MACAddress_P_& mac)
	{
		operator=(&mac);
	}

};


/// ARP packet layout
union ARPPacket
{
	struct
	{
		nint16_t HTYPE; //!< Hardware type. This field specifies the network protocol type. Example: Ethernet is 1.
		nint16_t PTYPE; //!< Protocol type. This field specifies the internetwork protocol for which the ARP request is intended. For IPv4, this has the value 0x0800. The permitted PTYPE values share a numbering space with those for EtherType.
		uint8_t HLEN; //!< Hardware address length. Length (in octets) of a hardware address. Ethernet addresses size is 6. 
		uint8_t PLEN; //!< Protocol address length. Length (in octets) of addresses used in the upper layer protocol. (The upper layer protocol specified in PTYPE.) IPv4 address size is 4.
		nint16_t OPER; //!< Operation. Specifies the operation that the sender is performing: 1 for request, 2 for reply.
		MACAddress senderMAC; //!< Sender hardware address. Media address of the sender. In an ARP request this field is used to indicate the address of the host sending the request. In an ARP reply this field is used to indicate the address of the host that the request was looking for. (Not necessarily address of the host replying as in the case of virtual media.) Note that switches do not pay attention to this field, particularly in learning MAC addresses.
		IPAddress senderIP; //!< Sender protocol address. Internet address of the sender.
		MACAddress targetMAC; //!< Target hardware address. Media address of the intended receiver. In an ARP request this field is ignored. In an ARP reply this field is used to indicate the address of the host that originated the ARP request.
		IPAddress targetIP; //!< Target protocol address. Internet address of the intended receiver.
	};
	uint8_t raw[28]; //!< byte-wise access to all fields
};

/// IP header 
union IPHeader
{
	struct
	{
		struct
		{
			uint8_t IHL : 4; //!< Internet Header Length. The second field (4 bits) is the Internet Header Length (IHL), which is the number of 32-bit words in the header. Since an IPv4 header may contain a variable number of options, this field specifies the size of the header (this also coincides with the offset to the data). The minimum value for this field is 5 (RFC 791), which is a length of 5×32 = 160 bits = 20 bytes. Being a 4-bit value, the maximum length is 15 words (15×32 bits) or 480 bits = 60 bytes.
			uint8_t version : 4; //!< Protocol version. The first header field in an IP packet is the four-bit version field. For IPv4, this has a value of 4 (hence the name IPv4).
		};
		struct
		{
			uint8_t ECN : 2; //!< Explicit Congestion Notification. This field is defined in RFC 3168 and allows end-to-end notification of network congestion without dropping packets. ECN is an optional feature that is only used when both endpoints support it and are willing to use it. It is only effective when supported by the underlying network.
			uint8_t DSCP : 6; //!< Differentiated Services Code Point
		};
		nint16_t totalLength; //!< This 16-bit field defines the entire packet (fragment) size, including header and data, in bytes. The minimum-length packet is 20 bytes (20-byte header + 0 bytes data) and the maximum is 65,535 bytes — the maximum value of a 16-bit word. All hosts are required to be able to reassemble datagrams of size up to 576 bytes, but most modern hosts handle much larger packets. Sometimes subnetworks impose further restrictions on the packet size, in which case datagrams must be fragmented. Fragmentation is handled in either the host or router in IPv4.
		nint16_t identification; //!< Identification. This field is an identification field and is primarily used for uniquely identifying the group of fragments of a single IP datagram.Some experimental work has suggested using the ID field for other purposes, such as for adding packet - tracing information to help trace datagrams with spoofed source addresses, [13] but RFC 6864 now prohibits any such use.
		struct
		{
			uint16_t fragmentOffset : 13; //!< Fragment Offset. The fragment offset field, measured in units of eight-byte blocks (64 bits), is 13 bits long and specifies the offset of a particular fragment relative to the beginning of the original unfragmented IP datagram. The first fragment has an offset of zero. This allows a maximum offset of (213 – 1) × 8 = 65,528 bytes, which would exceed the maximum IP packet length of 65,535 bytes with the header length included (65,528 + 20 = 65,548 bytes).
			uint16_t flags : 3; //!< Flags. A three-bit field follows and is used to control or identify fragments. They are (in order, from high order to low order): 	bit 0: Reserved; must be zero.[note 1]	bit 1: Don't Fragment (DF) bit 2 : More Fragments(MF)			If the DF flag is set, and fragmentation is required to route the packet, then the packet is dropped.This can be used when sending packets to a host that does not have sufficient resources to handle fragmentation.It can also be used for Path MTU Discovery, either automatically by the host IP software, or manually using diagnostic tools such as ping or traceroute.For unfragmented packets, the MF flag is cleared.For fragmented packets, all fragments except the last have the MF flag set.The last fragment has a non - zero Fragment Offset field, differentiating it from an unfragmented packet.
		};
		uint8_t TTL; //!< Time To Live. An eight-bit time to live field helps prevent datagrams from persisting (e.g. going in circles) on an internet. This field limits a datagram's lifetime. It is specified in seconds, but time intervals less than 1 second are rounded up to 1. In practice, the field has become a hop count—when the datagram arrives at a router, the router decrements the TTL field by one. When the TTL field hits zero, the router discards the packet and typically sends an ICMP Time Exceeded message to the sender.	The program traceroute uses these ICMP Time Exceeded messages to print the routers used by packets to go from the source to the destination.
		uint8_t protocol; //!< Protocol. This field defines the protocol used in the data portion of the IP datagram. The Internet Assigned Numbers Authority maintains a list of IP protocol numbers which was originally defined in RFC 790.
		nint16_t checksum; //!< Header Checksum. The 16-bit checksum field is used for error-checking of the header. When a packet arrives at a router, the router calculates the checksum of the header and compares it to the checksum field. If the values do not match, the router discards the packet. Errors in the data field must be handled by the encapsulated protocol. Both UDP and TCP have checksum fields. When a packet arrives at a router, the router decreases the TTL field. Consequently, the router must calculate a new checksum. RFC 1071 defines the checksum calculation.
		IPAddress sourceIP; //!< Source address. This field is the IPv4 address of the sender of the packet. Note that this address may be changed in transit by a network address translation device.
		IPAddress destinationIP; //!< Destination address. This field is the IPv4 address of the receiver of the packet. As with the source address, this may be changed in transit by a network address translation device.
	};
	uint8_t raw[20]; //!< byte-wise access to all fields

};

/// TCP flags data structure
union TCPFlags
{
	struct
	{
		uint8_t FIN : 1; //!< No more data from sender
		uint8_t SYN : 1; //!< Synchronize sequence numbers. Only the first packet sent from each end should have this flag set. Some other flags and fields change meaning based on this flag, and some are only valid for when it is set, and others when it is clear.
		uint8_t RST : 1; //!< Reset the connection
		uint8_t PSH : 1; //!< Push function. Asks to push the buffered data to the receiving application.
		uint8_t ACK : 1; //!< Indicates that the Acknowledgment field is significant. All packets after the initial SYN packet sent by the client should have this flag set.
		uint8_t URG : 1; //!< Indicates that the Urgent pointer field is significant
		uint8_t ECE : 1; //!< ECN-Echo has a dual role, depending on the value of the SYN flag. It indicates: If the SYN flag is set(1), that the TCP peer is ECN capable. If the SYN flag is clear(0), that a packet with Congestion Experienced flag in IP header set is received during normal transmission(added to header by RFC 3168). 
		uint8_t CWR : 1; //!< Congestion Window Reduced (CWR) flag is set by the sending host to indicate that it received a TCP segment with the ECE flag set and had responded in congestion control mechanism (added to header by RFC 3168).
	};
	uint8_t raw; //!< byte-wise access to all flags

	/// <summary>
	/// Sets all flags to zero.
	/// </summary>
	void clear()
	{
		raw = 0;
	}
};

/// TCP Header data structure
struct TCPHeader
{
	nint16_t sourcePort; //!< identifies the sending port
	nint16_t destinationPort; //!< identifies the receiving port
	nint32_t sequenceNumber; //!< has a dual role: If the SYN flag is set(1), then this is the initial sequence number.The sequence number of the actual first data byte and the acknowledged number in the corresponding ACK are then this sequence number plus 1. If the SYN flag is clear(0), then this is the accumulated sequence number of the first data byte of this segment for the current session.
	nint32_t acknowledgementNumber; //!< if the ACK flag is set then the value of this field is the next sequence number that the receiver is expecting. This acknowledges receipt of all prior bytes (if any). The first ACK sent by each end acknowledges the other end's initial sequence number itself, but no data.

	union
	{
		struct
		{
			uint8_t NS : 1; //!< ECN-nonce concealment protection (experimental: see RFC 3540).
			uint8_t reserved : 3; //!< Reserved for future use
			uint8_t headerLength : 4; //!< 
			TCPFlags flags; //!< TCP Flags. see TCPFlags struct for more information
		};
		uint16_t allFlags; //!< byte-wise access to all flags.
	};

	nint16_t windowSize; //!< The size of the receive window, which specifies the number of window size units (by default, bytes) (beyond the sequence number in the acknowledgment field) that the sender of this segment is currently willing to receive
	nint16_t checksum; //!< The 16-bit checksum field is used for error-checking of the header and data
	nint16_t urgentPointer; //!< if the URG flag is set, then this 16-bit field is an offset from the sequence number indicating the last urgent data byte
};

/// Structure to encode one 2-byte long TCP option
struct TCPOptions
{

	uint8_t option1; //!< Option kind
	uint8_t option1_length; //!< option length
	nint16_t option1_value; //!< option value

};

/// Structure to represent an UDP header
struct UDPHeader
{
	nint16_t sourcePort; //!< Identifies the sending port
	nint16_t destinationPort; //!< Identifies the receiving port
	nint16_t dataLength; //!< Total length of payload and header
	nint16_t checksum; //!< The 16-bit checksum field is used for error-checking of the header and data
};

/// Structure used to encode part of a DNS query
struct DNSHeader
{
	uint16_t identification; //!< A 16-bit identification number chosen by the client. The server will reply with the same number so the client can match a response to a request.
	union
	{
		struct
		{
			uint8_t RD : 1; //!< Recursion Desired. When set in a query, requests that the server receiving the query attempt to answer the query recursively, if the server supports recursive resolution. The value of this bit is not changed in the response.
			uint8_t TC : 1; //!< Truncation Flag. When set to 1, indicates that the message was truncated due to its length being longer than the maximum permitted for the type of transport mechanism used. TCP doesn't have a length limit for messages, while UDP messages are limited to 512 bytes, so this bit being sent usually is an indication that the message was sent using UDP and was too long to fit. The client may need to establish a TCP session to get the full message. On the other hand, if the portion truncated was part of the Additional section, it may choose not to bother.
			uint8_t AA : 1; //!< Authoritative Answer Flag. This bit is set to 1 in a response to indicate that the server that created the response is authoritative for the zone in which the domain name specified in the Question section is located. If it is 0, the response is non-authoritative.
			uint8_t opcode : 4; //!< Operation Code.
			uint8_t QR : 1; //!< Query or Response. Query:0, Response:1
			uint8_t rcode : 4; //!< Response Code. This field indicates whether the query was successful (0) or not (nonzero)
			uint8_t reserved : 3;
			uint8_t RA : 1; //!< Recursion Available. Set to 1 or cleared to 0 in a response to indicate whether the server creating the response supports recursive queries. This can then be noted by the device that sent the query for future use.
		};
		uint16_t flags;
	};
	nint16_t numberOfQuestions; //!< Question Count. Specifies the number of questions in the Question section of the message.
	nint16_t numberOfAnswerRRs; //!< Answer Record Count. Specifies the number of resource records in the Answer section of the message.
	nint16_t numberOfAuthorityRRs; //!< Authority Record Count. Specifies the number of resource records in the Authority section of the message.
	nint16_t numberOfAdditionalRRs; //!< Additional Record Count. Specifies the number of resource records in the Additional section of the message.

	/// <summary>
	/// Sets the entire header to zero.
	/// </summary>
	void zero()
	{
		memset(this, 0, sizeof(DNSHeader));
	}

};

/// ICMP Header and Echo request/reply
struct ICMPHeader
{
	uint8_t type; //!< ICMP type
	uint8_t code; //!< ICMP subtype
	uint16_t checksum; //!< Error checking data. Calculated from the ICMP header and data, with value 0 substituted for this field. The Internet Checksum is used, specified in RFC 1071
	union
	{
		struct
		{
			uint16_t identifier; //!< used to identify this echo request
			uint16_t sequenceNumber; //!< used to identify this echo request
		};
		uint32_t timestamp; //!< convenient merge of both identifier and sequence number to use as storage for millis()
	};

	/// <summary>
	/// Points to the beginning of the ICMP echo data.
	/// </summary>
	/// <returns></returns>
	uint8_t* dataStart()
	{
		return (uint8_t*)this + sizeof(ICMPHeader);
	}
};

/// Represents the header of an Ethernet frame
struct EthernetHeader
{
	MACAddress dstMAC;  //!< Destination MAC address
	MACAddress srcMAC; //!< Source MAC address
	nint16_t etherType;  //!< Protocol type, e.g. IP, ARP...
};

/// Holds a DHCP header
struct DHCPHeader
{
	uint8_t op; //!< Operation Code. Specifies the general type of message. A value of 1 indicates a request message, while a value of 2 is a reply message.
	uint8_t htype; //!< Hardware Type. Type of hardware used in the network.
	uint8_t hlen; //!< Hardware Address Length. Specifies how long hardware addresses are in this message. For Ethernet or other networks using IEEE 802 MAC addresses, the value is 6
	uint8_t hops; //!< Hops. Set to 0 by a client before transmitting a request and used by relay agents to control the forwarding of BOOTP and/or DHCP messages
	nint32_t xid; //!< Transaction Identifier. A 32-bit identification field generated by the client, to allow it to match up the request with replies received from DHCP servers.
	nint16_t secs; //!< Number of seconds elapsed since a client began an attempt to acquire or renew a lease. This may be used by a busy DHCP server to prioritize replies when multiple client requests are outstanding.
	uint16_t broadcastFlag; //!< Broadcast flag. Set when the client doesn't know its own IP Address (server must respond via broadcast). Set to 0x0100.
	IPAddress ciaddr; //!< Client IP Address. The client puts its own current IP address in this field if and only if it has a valid IP address while in the BOUND, RENEWING or REBINDING states; otherwise, it sets the field to 0. The client can only use this field when its address is actually valid and usable, not during the process of acquiring an address
	IPAddress yiaddr; //!< Your IP Address. The IP address that the server is assigning to the client.
	IPAddress siaddr; //!< Server IP address:  the address of the server that the client should use for the next step in the bootstrap process, which may or may not be the server sending this reply.
	IPAddress giaddr; //!< Gateway for DHCP request routing
	union
	{
		MACAddress mac; //!< The hardware (layer two) address of the client, which is used for identification and communication.
		byte chaddr[16];
	};
	byte sname[64]; //!< Server Name. The server sending a DHCPOFFER or DHCPACK message may optionally put its name in this field. This can be a simple text “nickname” or a fully-qualified DNS domain name (such as “myserver.organization.org”).
	byte filename[128];
	IPAddress magicCookie;
};

/// struct to represent the remaining space for DHCP options
struct DHCPOptions
{
	byte data[EtherDune_BUFFER_SIZE - sizeof(DHCPHeader) - sizeof(UDPHeader) - sizeof(IPHeader) - sizeof(EthernetHeader)]; 
};

/// Union of all the different protocol headers and layers to help EtherDune interpret
/// or build packet.
union EthBuffer
{
	struct
	{
		EthernetHeader eth;
		union
		{
			ARPPacket arp;
			struct
			{
				IPHeader ip;
				union
				{
					ICMPHeader icmp;
					struct
					{
						UDPHeader udp;
						union
						{
							byte udpData[EtherDune_BUFFER_SIZE - sizeof(EthernetHeader) - sizeof(IPHeader) - sizeof(UDPHeader)];
							DNSHeader dns;
							struct
							{
								DHCPHeader dhcp;
								DHCPOptions dhcpOptions;
							};
						};
						
					};
					struct
					{
						TCPHeader tcp;
						TCPOptions tcpOptions;
					};

				};

			};

		};
	};
	uint8_t raw[EtherDune_BUFFER_SIZE];
};


/// Entry in the ARP table
struct ARPEntry
{
	IPAddress ip; //!< IP address
	MACAddress mac; //!< MAC address
	int8_t status_TTL; //!< TTL in minutes to consider the entry valid.
};


static const uint8_t SCK_STATE_CLOSED = 0; //!<  (both server and client) represents no connection state at all.
static const uint8_t SCK_STATE_LISTEN = 1; //!<  (server) represents waiting for a connection request from any remote TCP and port.
static const uint8_t SCK_STATE_SYN_SENT = 2; //!<  (client) represents waiting for a matching connection request after having sent a connection request.
static const uint8_t SCK_STATE_SYN_RECEIVED = 3; //!<  (server) represents waiting for a confirming connection request acknowledgment after having both received and sent a connection request.
static const uint8_t SCK_STATE_ESTABLISHED = 4; //!<  (both server and client) represents an open connection, data received can be delivered to the user. The normal state for the data transfer phase of the connection.
static const uint8_t SCK_STATE_FIN_WAIT_1 = 5; //!<  (both server and client) represents waiting for a connection termination request from the remote TCP, or an acknowledgment of the connection termination request previously sent.
static const uint8_t SCK_STATE_FIN_WAIT_2 = 6; //!<  (both server and client) represents waiting for a connection termination request from the remote TCP.
static const uint8_t SCK_STATE_CLOSE_WAIT = 7; //!<  (both server and client) represents waiting for a connection termination request from the local user.
static const uint8_t SCK_STATE_CLOSING = 8; //!<  (both server and client) represents waiting for a connection termination request acknowledgment from the remote TCP.
static const uint8_t SCK_STATE_LAST_ACK = 9; //!<  (both server and client) represents waiting for an acknowledgment of the connection termination request previously sent to the remote TCP (which includes an acknowledgment of its connection termination request).
static const uint8_t SCK_STATE_TIME_WAIT = 10; //!< (either server or client) represents waiting for enough time to pass to be sure the remote TCP received the acknowledgment of its connection termination request. [According to RFC 793 a connection can stay in TIME-WAIT for a maximum of four minutes known as a MSL (maximum segment lifetime).]
static const uint8_t SCK_STATE_RESOLVING = 11; //!< IP Address is being resolved

static const uint8_t DHCP_STATE_INIT = 0; //!< This is the initialization state, where a client begins the process of acquiring a lease. It also returns here when a lease ends, or when a lease negotiation fails.
static const uint8_t DHCP_STATE_SELECTING = 1; //!< The client is waiting to receive DHCPOFFER messages from one or more DHCP servers, so it can choose one.
static const uint8_t DHCP_STATE_REQUESTING = 2; //!< The client is waiting to hear back from the server to which it sent its request.
static const uint8_t DHCP_STATE_BOUND = 3; //!< Client has a valid lease and is in its normal operating state.
static const uint8_t DHCP_STATE_RENEWING = 4; //!< Client is trying to renew its lease. It regularly sends DHCPREQUEST messages with the server that gave it its current lease specified, and waits for a reply.
static const uint8_t DHCP_STATE_REBINDING = 5; //!< The client has failed to renew its lease with the server that originally granted it, and now seeks a lease extension with any server that can hear it. It periodically sends DHCPREQUEST messages with no server specified until it gets a reply or the lease ends.

static const uint8_t IP_PROTO_ICMP = 1; //!< IP header protocol type for ICMP
static const uint8_t IP_PROTO_TCP = 6; //!< IP header protocol type for TCP
static const uint8_t IP_PROTO_UDP = 17; //!< IP header protocol type for UDP

static const uint8_t ICMP_TYPE_ECHOREPLY = 0; //!< ICMP echo reply operation type
static const uint8_t ICMP_TYPE_ECHOREQUEST = 8; //!< ICMP echo request operation type

static const uint16_t ETHTYPE_ARP = 0x0806; //!< Ethernet header protocol type for ARP
static const uint16_t ETHTYPE_IP = 0x0800; //!< Ethernet header protocol type for IP

static const uint8_t ARP_OPCODE_REPLY_L = 0x02; //!< ARP Opcode for reply
static const uint8_t ARP_OPCODE_REQ_L = 0x01; //!< ARP Opcode for request

#endif