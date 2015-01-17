#ifndef __inet_h_
#define __inet_h_
#if ARDUINO >= 100
#include <Arduino.h> // Arduino 1.0
#else
#include <WProgram.h> // Arduino 0022
#endif

#include <avr/pgmspace.h>

static const uint16_t ETHTYPE_ARP = 0x0806;

static const uint8_t ARP_OPCODE_REPLY_L = 0x02;


typedef union u16_t
{
	struct
	{
		uint8_t l;
		uint8_t h;
	};
	uint16_t v;
};


typedef union nint16_t
{
	uint8_t raw[2];
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
		u16_t u;
		u.v = v;
		l = u.l;
		h = u.h;
	}
};

typedef struct nint32_t
{
	nint16_t h;
	nint16_t l;
};

typedef union IPAddress
{
	uint8_t b[4];
	nint32_t t;
	uint32_t u;

	void set(IPAddress& ip)
	{
		memcpy(b, &ip, sizeof(IPAddress));
	}
	void set_P( PGM_VOID_P ip)
	{
		memcpy_P (b, ip, sizeof(IPAddress));
	}
};


typedef struct MACAddress
{
	uint8_t b[6];

	void set(MACAddress& mac)
	{
		memcpy(b, &mac, sizeof(MACAddress));
	}

	void set_P( PGM_VOID_P mac)
	{
		memcpy_P (b, mac, sizeof(MACAddress));
	}
};


typedef union ARPPacket
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

typedef struct IPHeader
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
		uint8_t flags : 3;
	};
	uint8_t TTL;
	uint8_t protocol;
	nint16_t checksum;
	IPAddress sourceIP;
	IPAddress destinationIP;

};

typedef struct TCPHeader
{
	nint16_t sourcePort;
	nint16_t destinationPort;
	nint32_t sequenceNumber;
	nint32_t acknowledgementNumber;
	struct
	{
		union
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
			uint8_t flags;
		};
		uint8_t NS : 1;
		uint8_t reserved : 3;
		uint8_t dataOffset : 4;
	};
	nint16_t windowSize;
	nint16_t checksum;
	nint16_t urgentPointer;
	struct options
	{
		uint8_t option1;
		uint8_t option1_length;
		nint16_t option1_value;
	};
};

typedef struct ICMPHeader
{
	uint8_t type;
	uint8_t code;
	uint8_t checksum;
	uint8_t rest[4];
};

typedef union EthBuffer
{
	struct
	{
		MACAddress dstMAC;
		MACAddress srcMAC;
		nint16_t etherType;
		union
		{
			ARPPacket arp;
			struct
			{
				IPHeader ip;
				union
				{
					ICMPHeader icmp;
					TCPHeader tcp;
				};
			};

		};
	};
	uint8_t raw[64];
};



typedef struct ARPEntry
{
	IPAddress ip;
	MACAddress mac;
	int16_t status_TTL; 
};







#endif