#ifndef __ETHERSOCKET_CONFIG__
#define __ETHERSOCKET_CONFIG__
// Ethersocket configuration


// ENC28J60 memory mapping

// The RXSTART_INIT must be zero. See Rev. B4 Silicon Errata point 5.
// Buffer boundaries applied to internal 8K ram
// the entire available packet buffer space is allocated

#define RXSTART_INIT        0x0000  // start of RX buffer, room for 2 packets
#define RXSTOP_INIT         0x0BFF  // end of RX buffer

#define TXSTART_INIT        (RXSTOP_INIT+1)  // start of TX buffer
#define TXSTOP_INIT         (ENC28J60_MEMSIZE-1)  // end of TX buffer

static const uint8_t SLOT_SIZE_SHIFT = 9;

//indicate how to distribute buffer slots. There are up to NUM_SLOTS slots
//NUM_SLOTS is 10 by default.
// slot #0 is reserved for ARP requests
#define MAX_TCP_SOCKETS = 8;
#define MAX_UDP_SOCKETS = 1;



// max frame length which the conroller will accept:
// (note: maximum ethernet frame length would be 1518)
#define MAX_FRAMELEN      1500
#define FULL_SPEED  1   // switch to full-speed SPI for bulk transfers



//handy formulas using the above
static const uint16_t SLOT_SIZE = 1 << SLOT_SIZE_SHIFT;
static const uint16_t NUM_SLOTS = (ENC28J60_MEMSIZE - (RXSTOP_INIT - RXSTART_INIT + 1)) / SLOT_SIZE;
#define SLOT_ADDR(x) (TXSTART_INIT + (x << SLOT_SIZE_SHIFT ))



#endif