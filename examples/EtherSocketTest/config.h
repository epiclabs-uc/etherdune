#ifndef __ETHERSOCKET_CONFIG__
#define __ETHERSOCKET_CONFIG__
// Ethersocket configuration


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

static const uint8_t SLOT_SIZE_SHIFT = 8;

//indicate how to distribute buffer slots. There are up to NUM_SLOTS slots
//NUM_SLOTS is 10 by default.
// slot #0 is reserved for ARP requests
static const uint8_t MAX_TCP_SOCKETS = 8;
static const uint8_t MAX_UDP_SOCKETS = 1;
static const uint8_t TCP_SRC_PORT_H = 250;
static const uint16_t TCP_MAXIMUM_SEGMENT_SIZE = 512;

static const uint16_t NETWORK_TIMER_RESOLUTION = 200; //ms
static const uint8_t ARP_TABLE_LENGTH = 2;
static const int16_t MAX_ARP_TTL = 20; // 20 mins


// max frame length which the conroller will accept:
// (note: maximum ethernet frame length would be 1518)
#define MAX_FRAMELEN      1500
#define FULL_SPEED  1   // switch to full-speed SPI for bulk transfers



//handy formulas using the above
static const uint16_t SLOT_SIZE = 1 << SLOT_SIZE_SHIFT;
static const uint16_t NUM_SLOTS = (ENC28J60_MEMSIZE - TXBUFFER_SIZE - (RXSTOP_INIT - RXSTART_INIT + 1)) / SLOT_SIZE;
#define SLOT_ADDR(x) (TXSTOP_INIT + 1 + (x << SLOT_SIZE_SHIFT ))

static const uint16_t TXSTART_INIT_DATA = TXSTART_INIT + 1; // skip 1 byte to make room for the control byte required by ENC28J60


#endif