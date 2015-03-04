#ifndef __EtherFlow_CONFIG__
#define __EtherFlow_CONFIG__

#include <ACross.h>

// EtherFlow configuration

//RAM memory buffer size to hold a packet.
//default is 566 which is enough to hold a TCP packet of 512 bytes
#define ETHERFLOW_BUFFER_SIZE 566   


//ETHERFLOW_SAMPLE_SIZE: minimum amount of bytes to read to decide whether we want the rest of the packet or not
//There is a lot of cosmic noise in an Ethernet network, broadcasts, etc that we can filter out using this feature.
//with 42 bytes we can read an entire ARP packet in the sample
// + we get IP and UDP full headers as well as TCP sourcePort, localPort and sequenceNumber
//sizeof(EthernetHeader) = 14
//sizeof(ARPPacket) = 28
//total = 42.

//sizeof(EtheretHeader) = 14
//sizeof(IPHeader) = 20
//sizeof(sourcePort) = 2
//sizeof(destinationPort)= 2
//sizeof(sequenceNumber) = 4
//total = 42

//to disable sampling, set ETHERFLOW_SAMPLE_SIZE to ETHERFLOW_BUFFER_SIZE

#define ETHERFLOW_SAMPLE_SIZE 42  

//Checksum options

#define ENABLE_IP_RX_CHECKSUM true // enabling this will drop packets that have checksum errors in the IP header
#define ENABLE_UDPTCP_RX_CHECKSUM true // enabling this will drop TCP/UDP packets that have checksum errors

// ENC28J60 memory mapping
#define ENC28J60_MEMSIZE  8192 //don't change this, that's just how much RAM there is in your ENC28J60.

// The RXSTART_INIT must be zero. See Rev. B4 Silicon Errata point 5.
// Buffer boundaries applied to internal 8K ram
// the entire available packet buffer space is allocated

static const uint16_t RXSTART_INIT =         0x0000;  // start of RX buffer, room for 2 packets
static const uint16_t RXSTOP_INIT=          0x0BFF;  // end of RX buffer. Make sure RXSTOP_INIT is an odd number due to hardware bugs.

static const uint16_t TXBUFFER_SIZE = 1024;
static const uint16_t TXSTART_INIT = (RXSTOP_INIT + 1);  // start of TX buffer
static const uint16_t TXSTOP_INIT = (TXSTART_INIT + TXBUFFER_SIZE - 1);  // end of TX buffer

static const uint16_t SHARED_BUFFER_INIT = TXSTOP_INIT + 1;
static const uint16_t SHARED_BUFFER_CAPACITY = ENC28J60_MEMSIZE - SHARED_BUFFER_INIT;

static const uint8_t TCP_SRC_PORT_H = 250;
static const uint8_t UDP_SRC_PORT_H = 240;

static const uint16_t TCP_MAXIMUM_SEGMENT_SIZE = 512;

static const uint16_t NETWORK_TIMER_RESOLUTION = 1000; //ms
static const uint8_t ARP_TABLE_LENGTH = 2;
static const int16_t MAX_ARP_TTL = 20; // 20 mins

static const uint8_t MAX_TCP_CONNECT_RETRIES = 50;

// max frame length which the conroller will accept:
// (note: maximum ethernet frame length would be 1518)
#define MAX_FRAMELEN      1500
#define FULL_SPEED  1   // switch to full-speed SPI for bulk transfers

#define NTICKS(ms) ((ms)/NETWORK_TIMER_RESOLUTION)

// TCP timeouts, in milliseconds
// These represent how much patience will EtherFlow have waiting in this state before
// thinking something is wrong and doing something about it.
static const uint8_t SCK_TIMEOUT_SYN_SENT = NTICKS(5000); // (client) represents waiting for a matching connection request after having sent a connection request.
static const uint8_t SCK_TIMEOUT_SYN_RECEIVED = NTICKS(5000); //(server) SYN has been received, SYN ACK was sent and now waiting for ACK.
static const uint8_t SCK_TIMEOUT_FIN_WAIT_1 = NTICKS(2000); // (both server and client) represents waiting for a connection termination request from the remote TCP, or an acknowledgment of the connection termination request previously sent.
static const uint8_t SCK_TIMEOUT_FIN_WAIT_2 = NTICKS(2000); // (both server and client) represents waiting for a connection termination request from the remote TCP.
static const uint8_t SCK_TIMEOUT_CLOSE_WAIT = NTICKS(2000); // (both server and client) represents waiting for a connection termination request from the local user.
static const uint8_t SCK_TIMEOUT_CLOSING = NTICKS(2000); // (both server and client) represents waiting for a connection termination request acknowledgment from the remote TCP.
static const uint8_t SCK_TIMEOUT_LAST_ACK = NTICKS(2000); // (both server and client) represents waiting for an acknowledgment of the connection termination request previously sent to the remote TCP (which includes an acknowledgment of its connection termination request).
static const uint8_t SCK_TIMEOUT_TIME_WAIT = NTICKS(2000); //(either server or client) represents waiting for enough time to pass to be sure the remote TCP received the acknowledgment of its connection termination request. [According to RFC 793 a connection can stay in TIME-WAIT for a maximum of four minutes known as a MSL (maximum segment lifetime).]
static const uint8_t SCK_TIMEOUT_RESOLVING = NTICKS(3000); // waiting for a DNS query to resolve

static const uint8_t DNS_TIMEOUT_QUERY = NTICKS(5000);//DNS timeout


//DHCP timeouts, in milliseconds
static const uint8_t DHCP_TIMEOUT_SELECTING = NTICKS(5000);
static const uint8_t DHCP_TIMEOUT_REQUESTING = NTICKS(5000);
static const uint8_t DHCP_TIMEOUT_RENEWING = NTICKS(5000);
static const uint8_t DHCP_TIMEOUT_REBINDING = NTICKS(5000);


static const uint16_t TXSTART_INIT_DATA = TXSTART_INIT + 1; // skip 1 byte to make room for the control byte required by ENC28J60

#define CLOCK(id,x) unsigned long clock##id=millis();x;dsprint("" #x ": " );dprint(millis() - clock##id);dsprintln(" ms.")
#define PRINTSIZEOF(thing) AC_DEBUG(dsprint("sizeof(" #thing ")=");dprintln(sizeof(thing)))


//error checking

#if ETHERFLOW_BUFFER_SIZE < 566
#error ETHERFLOW_BUFFER_SIZE must be at least 566 bytes
#endif

//see ETHERFLOW_SAMPLE_SIZE definition above to see why 42
#if ETHERFLOW_SAMPLE_SIZE < 42
#error ETHERFLOW_SAMPLE_SIZE must be at least 42 bytes
#endif

#if ETHERFLOW_SAMPLE_SIZE > ETHERFLOW_BUFFER_SIZE
#error ETHERFLOW_SAMPLE_SIZE must be less than ETHERFLOW_BUFFER_SIZE
#endif

#if ETHERFLOW_BUFFER_SIZE & 1
#error ETHERFLOW_BUFFER_SIZE must be an even number
#endif


#endif