#ifndef __EtherFlow_CONFIG__
#define __EtherFlow_CONFIG__

#include <ACross.h>

// EtherFlow configuration

#define EtherFlow_DEBUG 1



#if EtherFlow_DEBUG
	#define DEBUG(x) x
#else
	#define DEBUG(x)
#endif



#define CLOCK(id,x) unsigned long clock##id=millis();x;Serial.print("" #x ": " );Serial.print(millis() - clock##id);Serial.println(" ms.")


// ENC28J60 memory mapping
#define ENC28J60_MEMSIZE  8192

// The RXSTART_INIT must be zero. See Rev. B4 Silicon Errata point 5.
// Buffer boundaries applied to internal 8K ram
// the entire available packet buffer space is allocated

static const uint16_t RXSTART_INIT =         0x0000;  // start of RX buffer, room for 2 packets
static const uint16_t RXSTOP_INIT=          0x0BFF;  // end of RX buffer

static const uint16_t TXBUFFER_SIZE = 1024;
static const uint16_t TXSTART_INIT = (RXSTOP_INIT + 1);  // start of TX buffer
static const uint16_t TXSTOP_INIT = (TXSTART_INIT + TXBUFFER_SIZE - 1);  // end of TX buffer

static const uint16_t SHARED_BUFFER_INIT = TXSTOP_INIT + 1;
static const uint16_t SHARED_BUFFER_CAPACITY = ENC28J60_MEMSIZE - SHARED_BUFFER_INIT;

static const uint8_t SLOT_SIZE_SHIFT = 9; //slot size = 2^SLOT_SIZE_SHIFT, 512 bytes by default.
static const uint8_t MAX_SLOTS_PER_SOCKET = 4;


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

// TCP timeouts
static const uint8_t SCK_TIMEOUT_SYN_SENT = NTICKS(5000); // (client) represents waiting for a matching connection request after having sent a connection request.
static const uint8_t SCK_TIMEOUT_FIN_WAIT_1 = NTICKS(2000); // (both server and client) represents waiting for a connection termination request from the remote TCP, or an acknowledgment of the connection termination request previously sent.
static const uint8_t SCK_TIMEOUT_FIN_WAIT_2 = NTICKS(2000); // (both server and client) represents waiting for a connection termination request from the remote TCP.
static const uint8_t SCK_TIMEOUT_CLOSE_WAIT = NTICKS(2000); // (both server and client) represents waiting for a connection termination request from the local user.
static const uint8_t SCK_TIMEOUT_CLOSING = NTICKS(2000); // (both server and client) represents waiting for a connection termination request acknowledgment from the remote TCP.
static const uint8_t SCK_TIMEOUT_LAST_ACK = NTICKS(2000); // (both server and client) represents waiting for an acknowledgment of the connection termination request previously sent to the remote TCP (which includes an acknowledgment of its connection termination request).
static const uint8_t SCK_TIMEOUT_TIME_WAIT = NTICKS(2000); //(either server or client) represents waiting for enough time to pass to be sure the remote TCP received the acknowledgment of its connection termination request. [According to RFC 793 a connection can stay in TIME-WAIT for a maximum of four minutes known as a MSL (maximum segment lifetime).]
static const uint8_t SCK_TIMEOUT_RESOLVING = NTICKS(3000); // waiting for a DNS query to resolve
//DNS timeout

static const uint8_t DNS_TIMEOUT_QUERY = NTICKS(2000);




//handy formulas using the above
static const uint16_t SLOT_SIZE = 1 << SLOT_SIZE_SHIFT;
static const uint16_t NUM_SLOTS = (ENC28J60_MEMSIZE - TXBUFFER_SIZE - (RXSTOP_INIT - RXSTART_INIT + 1)) / SLOT_SIZE;
#define SLOT_ADDR(x) (TXSTOP_INIT + 1 + (x << SLOT_SIZE_SHIFT ))

static const uint16_t TXSTART_INIT_DATA = TXSTART_INIT + 1; // skip 1 byte to make room for the control byte required by ENC28J60


#endif