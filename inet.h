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


union nint16_t
{
	uint8_t raw[2];
	uint16_t rawu;
	struct
	{
		uint8_t h;
		uint8_t l;
	};

	uint16_t getValue()
	{
		u16_t u;
		u.l = l;
		u.h = h;

		return u.v;
	}
	void setValue(uint16_t v)
	{

		l = (uint8_t)v;
		h = v >> 8;
	}

	inline void zero()
	{
		rawu = 0;
	}
};

union nint32_t
{
	uint32_t rawu;
	struct
	{
		nint16_t h;
		nint16_t l;
	};
	uint8_t raw[4];


	uint32_t getValue()
	{
		u32_t u;
		u.h.h = h.h;
		u.h.l = h.l;
		u.l.h = l.h;
		u.l.l = l.l;
		return u.v;
	}

	void setValue(uint32_t v)
	{
		u32_t u;
		u.v = v;

		h.h = u.h.h;
		h.l = u.h.l;
		l.h = u.l.h;
		l.l = u.l.l;



	}

	inline void zero()
	{
		rawu = 0;
	}

};

struct IPAddress_P_
{
	uint8_t b[4];
};

#define IPAddress_P PROGMEM IPAddress_P_ 

#ifdef ACROSS_ARDUINO
#define IPADDR_P(b0,b1,b2,b3) (__extension__({static IPAddress_P_ __c PROGMEM = {(b0),(b1),(b2),(b3)}; &__c;}))
#else
#define IPADDR_P(b0,b1,b2,b3) {{ (b0),(b1),(b2),(b3) }}
#endif

union IPAddress
{
	uint8_t b[4];
	nint32_t t;
	uint32_t u;

	void operator = (IPAddress_P_* ip)
	{
		memcpy_P(b, ip, sizeof(IPAddress));
	}

	void operator = (IPAddress_P_& ip)
	{
		operator=(&ip);
	}

	//convert IP to string, for debugging purposes only.
	//if this function is used anywhere in your code, it will permanently allocate 16 bytes of RAM
	char* toString() const
	{
		static char addressString[16];
		sprintf(addressString, "%d.%d.%d.%d", b[0], b[1], b[2], b[3]);
		return addressString;
	}

};


struct MACAddress_P_
{
	uint8_t b[6];
};

#define MACAddress_P PROGMEM MACAddress_P_ 
#ifdef ACROSS_ARDUINO
#define MACADDR_P(b0,b1,b2,b3,b4,b5) (__extension__({static MACAddress_P_ __c PROGMEM = {(b0),(b1),(b2),(b3),(b4),(b5)}; &__c;}))
#else
#define MACADDR_P(b0,b1,b2,b3,b4,b5) {{ (b0),(b1),(b2),(b3),(b4),(b5) }}
#endif

struct MACAddress
{
	uint8_t b[6];

	void operator = (MACAddress_P_* mac)
	{
		memcpy_P(b, mac, sizeof(MACAddress));
	}

	void operator = (MACAddress_P_& mac)
	{
		operator=(&mac);
	}

};



union ARPPacket
{
	struct
	{
		nint16_t HTYPE;
		nint16_t PTYPE;
		uint8_t HLEN;
		uint8_t PLEN;
		nint16_t OPER;
		MACAddress senderMAC;
		IPAddress senderIP;
		MACAddress targetMAC;
		IPAddress targetIP;
	};
	uint8_t raw[28];
};

union IPHeader
{
	struct
	{
		struct
		{
			uint8_t IHL : 4;
			uint8_t version : 4;
		};
		struct
		{
			uint8_t ECN : 2;
			uint8_t DSCP : 6;
		};
		nint16_t totalLength;
		nint16_t identification;
		struct
		{
			uint16_t fragmentOffset : 13;
			uint16_t flags : 3;
		};
		uint8_t TTL;
		uint8_t protocol;
		nint16_t checksum;
		IPAddress sourceIP;
		IPAddress destinationIP;
	};
	uint8_t raw[20];

};

union TCPFlags
{
	struct
	{
		uint8_t FIN : 1;
		uint8_t SYN : 1;
		uint8_t RST : 1;
		uint8_t PSH : 1;
		uint8_t ACK : 1;
		uint8_t URG : 1;
		uint8_t ECE : 1;
		uint8_t CWR : 1;
	};
	uint8_t raw;

	void clear()
	{
		raw = 0;
	}
};

struct TCPHeader
{
	nint16_t sourcePort;
	nint16_t destinationPort;
	nint32_t sequenceNumber;
	nint32_t acknowledgementNumber;

	union
	{
		struct
		{
			uint8_t NS : 1;
			uint8_t reserved : 3;
			uint8_t headerLength : 4;
			TCPFlags flags;
		};
		uint16_t allFlags;
	};

	nint16_t windowSize;
	nint16_t checksum;
	nint16_t urgentPointer;
};

struct TCPOptions
{

	uint8_t option1;
	uint8_t option1_length;
	nint16_t option1_value;

};

struct UDPHeader
{
	nint16_t sourcePort;
	nint16_t destinationPort;
	nint16_t dataLength;
	nint16_t checksum;
};

struct DNSHeader
{
	uint16_t identification;
	union
	{
		struct
		{
			uint8_t RD : 1;
			uint8_t TC : 1;
			uint8_t AA : 1;
			uint8_t opcode : 4;
			uint8_t QR : 1;
			uint8_t rcode : 4;
			uint8_t reserved : 3;
			uint8_t RA : 1;
		};
		uint16_t flags;
	};
	nint16_t numberOfQuestions;
	nint16_t numberOfAnswerRRs;
	nint16_t numberOfAuthorityRRs;
	nint16_t numberOfAdditionalRRs;

	void zero()
	{
		memset(this, 0, sizeof(DNSHeader));
	}

};

struct ICMPHeader
{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	union
	{
		struct
		{
			uint16_t identifier;
			uint16_t sequenceNumber;
		};
		uint32_t timestamp;
	};
};

struct EthernetHeader
{
	MACAddress dstMAC;
	MACAddress srcMAC;
	nint16_t etherType;
};

struct DHCPHeader
{
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	nint32_t xid;
	nint16_t secs;
	uint16_t broadcastFlag; // 0x0100 //broadcast flag, set when the client doesn't know its own IP Address (server must respond via broadcast)
	IPAddress ciaddr;
	IPAddress yiaddr;
	IPAddress siaddr;
	IPAddress giaddr;
	union
	{
		MACAddress mac;
		byte chaddr[16];
	};
	byte sname[64];
	byte filename[128];
	IPAddress magicCookie;
};

struct DHCPOptions
{
	byte data[ETHERFLOW_BUFFER_SIZE - sizeof(DHCPHeader) - sizeof(UDPHeader) - sizeof(IPHeader) - sizeof(EthernetHeader)]; 
};


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
							byte udpData[ETHERFLOW_BUFFER_SIZE - sizeof(EthernetHeader) - sizeof(IPHeader) - sizeof(UDPHeader)];
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
	uint8_t raw[ETHERFLOW_BUFFER_SIZE];
};



struct ARPEntry
{
	IPAddress ip;
	MACAddress mac;
	int8_t status_TTL;
};


static const uint8_t SCK_STATE_CLOSED = 0; // (both server and client) represents no connection state at all.
static const uint8_t SCK_STATE_LISTEN = 1; // (server) represents waiting for a connection request from any remote TCP and port.
static const uint8_t SCK_STATE_SYN_SENT = 2; // (client) represents waiting for a matching connection request after having sent a connection request.
static const uint8_t SCK_STATE_SYN_RECEIVED = 3; // (server) represents waiting for a confirming connection request acknowledgment after having both received and sent a connection request.
static const uint8_t SCK_STATE_ESTABLISHED = 4; // (both server and client) represents an open connection, data received can be delivered to the user. The normal state for the data transfer phase of the connection.
static const uint8_t SCK_STATE_FIN_WAIT_1 = 5; // (both server and client) represents waiting for a connection termination request from the remote TCP, or an acknowledgment of the connection termination request previously sent.
static const uint8_t SCK_STATE_FIN_WAIT_2 = 6; // (both server and client) represents waiting for a connection termination request from the remote TCP.
static const uint8_t SCK_STATE_CLOSE_WAIT = 7; // (both server and client) represents waiting for a connection termination request from the local user.
static const uint8_t SCK_STATE_CLOSING = 8; // (both server and client) represents waiting for a connection termination request acknowledgment from the remote TCP.
static const uint8_t SCK_STATE_LAST_ACK = 9; // (both server and client) represents waiting for an acknowledgment of the connection termination request previously sent to the remote TCP (which includes an acknowledgment of its connection termination request).
static const uint8_t SCK_STATE_TIME_WAIT = 10; //(either server or client) represents waiting for enough time to pass to be sure the remote TCP received the acknowledgment of its connection termination request. [According to RFC 793 a connection can stay in TIME-WAIT for a maximum of four minutes known as a MSL (maximum segment lifetime).]
static const uint8_t SCK_STATE_RESOLVING = 11; //IP Address is being resolved

static const uint8_t DHCP_STATE_INIT = 0;
static const uint8_t DHCP_STATE_SELECTING = 1;
static const uint8_t DHCP_STATE_REQUESTING = 2;
static const uint8_t DHCP_STATE_BOUND = 3;
static const uint8_t DHCP_STATE_RENEWING = 4;
static const uint8_t DHCP_STATE_REBINDING = 5;

static const uint8_t IP_PROTO_ICMP = 1;
static const uint8_t IP_PROTO_TCP = 6;
static const uint8_t IP_PROTO_UDP = 17;

static const uint8_t ICMP_TYPE_ECHOREPLY = 0;
static const uint8_t ICMP_TYPE_ECHOREQUEST = 8;

static const uint16_t ETHTYPE_ARP = 0x0806;
static const uint16_t ETHTYPE_IP = 0x0800;

static const uint8_t ARP_OPCODE_REPLY_L = 0x02;
static const uint8_t ARP_OPCODE_REQ_L = 0x01;

#endif