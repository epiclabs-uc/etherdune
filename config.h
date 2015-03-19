// EtherFlow configuration file
// Author: Javier Peletier <jm@friendev.com>
// Summary: Etherflow settings, buffer sizes, compilation options, etc
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

#ifndef __EtherFlow_CONFIG__
#define __EtherFlow_CONFIG__

#include <ACross.h>

/// @defgroup config_h Etherflow configuration
/// All Etherflow settings, buffer sizes, compilation options, etc
///@{

/// <summary>Resolution of the NetworkService timer. All network services
/// (instances of the NetworkService class) receive a call to tick()
/// each time this amount of milliseconds is ellapsed. 
/// The recommended default is 200ms.
/// </summary>
static const uint16_t NETWORK_TIMER_RESOLUTION = 200;


/// @defgroup config_buffers Packet buffer configuration
/// Definition of buffer sizes
///@{

/// <summary>
/// RAM memory buffer size to hold a packet.
/// default is 566 which is enough to hold a TCP packet of 512 bytes
/// </summary>
#define ETHERFLOW_BUFFER_SIZE 566 


/// <summary>
/// Minimum amount of bytes to read to decide whether we want the rest of the packet or not.
/// There is a lot of cosmic noise in an Ethernet network, broadcasts, etc that we can filter out using this feature.
/// With 42 bytes we can read an entire ARP packet in the sample.
/// plus we get IP and UDP full headers as well as TCP sourcePort, localPort and sequenceNumber,
///  or even full ICMPHeader
/// 
/// Data Entity    |   Size  
/// -------------- | ------
/// EthernetHeader |  14
/// ARPPacket      |  28
///  TOTAL         |  42
/// 
/// Data Entity    |   Size  
/// -------------- | ------
/// EthernetHeader |  14
/// IPHeader       |  20
/// sourcePort     |  2
/// sequenceNumber |  4
///  TOTAL         |  42
/// 
/// Data Entity    |   Size  
/// -------------- | ------
/// EthernetHeader |  14
/// IPHeader       |  20
/// ICMPHeader     |  8
///  TOTAL         |  42
/// 
///
/// To disable sampling, set ETHERFLOW_SAMPLE_SIZE to ETHERFLOW_BUFFER_SIZE
/// </summary>
#define ETHERFLOW_SAMPLE_SIZE 42  

///@}

/// @defgroup checksumConfig Checksum configuration
/// Options to enable/disable checksum checking throughout EtherFlow
///@{

/// <summary>
/// Whether or not to check the integrity of the IP header.
/// Enabling this will drop packets that have checksum errors in the IP header
/// </summary>
#define ENABLE_IP_RX_CHECKSUM true

/// <summary>
/// Whether or not to check the integrity of TCP and UDP packets.
/// Enabling this will drop packets that have checksum errors.
/// </summary>
#define ENABLE_UDPTCP_RX_CHECKSUM true

/// <summary>
/// Whether or not to check the integrity of %ICMP packets.
/// Enabling this will drop packets that have checksum errors.
/// </summary>
#define ENABLE_ICMP_RX_CHECKSUM true 

///@}

/// @defgroup ENC28J60Config ENC28J60 Hardware configuration
/// %ENC28J60 memory segmentation and internal buffer sizes.
///
/// Buffer boundaries applied to internal 8K ram. The default configuration
/// allocates the entire available packet buffer space.
///
/// RX buffer: packets are received here in a circular buffer fashion.
/// The larger this buffer, the more packets that can be waiting for processing.
/// The default allows for approximately two ethernet frames. However, EtherFlow
/// will request the other end to send packets as small as possible given
/// memory is limited in the microcontroller.
///
/// TX buffer: space to assemble an outgoing packet.
/// The larger the buffer, the larger you can have Etherflow send TCP segments out.
/// You should not make this larger than 1500 which is the maximum size of an Ethernet frame anyway.
///
/// Shared buffer: This will automatically compute as the remainder: 8192 - RXBUFFER_SIZE - TXBUFFER_SIZE.
/// This is sort of a software-managed circular buffer shared among all sockets.
/// It holds TCP segments that may need to be retransmitted and serves as an assembly area when you call
/// write() on a socket. A minimum of 2048 bytes is recommended.
///@{

/// <summary> max frame length which the conroller will accept:
/// (note: maximum ethernet frame length would be 1518)
/// </summary>
#define MAX_FRAMELEN      1500

#define RXBUFFER_SIZE 3072 //!< size of the receive buffer size. This determines how large can an outgoing packet be. Make sure it is an even number.
#define TXBUFFER_SIZE 1024  //!< size of the transmit buffer size. This determines how large can an outgoing packet be.

///@}

/// @defgroup TCPUDPConfig TCP and UDP configuration
/// Default ports, timeouts, retries, etc.
///
///@{

/// <summary>Indicates the most significant byte of the source port number that Etherflow will
/// use by default when making outgoing connections. Each new socket will increment the least significant byte until it wraps around.
/// </summary>
static const uint8_t TCP_SRC_PORT_H = 250; 

/// <summary>Indicates the most significant byte of the source port number that Etherflow will
/// use by default when making outgoing connections. Each new UDP socket will increment the least significant byte until it wraps around.
/// </summary>
static const uint8_t UDP_SRC_PORT_H = 240;

static const uint16_t TCP_MAXIMUM_SEGMENT_SIZE = 512; //!< Maximum size of payload accepted by the library.
static const uint8_t MAX_TCP_CONNECT_RETRIES = 50; //!< how many SYN packets to send before giving up.


	/// @defgroup TCPTimings TCP state timeout configuration
	/// TCP timeouts, in milliseconds.
	/// These represent how much patience will EtherFlow have waiting in this state before
	/// thinking something is wrong and doing something about it.
	///@{
	#define SCK_TIMEOUT_SYN_SENT_MS  5000 //!< (client) represents waiting for a matching connection request after having sent a connection request.
	#define SCK_TIMEOUT_SYN_RECEIVED_MS  5000 //!< (server) SYN has been received, SYN ACK was sent and now waiting for ACK.
	#define SCK_TIMEOUT_FIN_WAIT_1_MS  6000 //!< (both server and client) represents waiting for a connection termination request from the remote TCP, or an acknowledgment of the connection termination request previously sent.
	#define SCK_TIMEOUT_FIN_WAIT_2_MS  2000 //!< (both server and client) represents waiting for a connection termination request from the remote TCP.
	#define SCK_TIMEOUT_CLOSE_WAIT_MS  2000 //!< (both server and client) represents waiting for a connection termination request from the local user.
	#define SCK_TIMEOUT_CLOSING_MS  2000 //!< (both server and client) represents waiting for a connection termination request acknowledgment from the remote TCP.
	#define SCK_TIMEOUT_LAST_ACK_MS  2000 //!< (both server and client) represents waiting for an acknowledgment of the connection termination request previously sent to the remote TCP (which includes an acknowledgment of its connection termination request).
	#define SCK_TIMEOUT_TIME_WAIT_MS  3000 //!<(either server or client) represents waiting for enough time to pass to be sure the remote TCP received the acknowledgment of its connection termination request. [According to RFC 793 a connection can stay in TIME-WAIT for a maximum of four minutes known as a MSL (maximum segment lifetime)].
	#define SCK_TIMEOUT_RESOLVING_MS  3000 //!< waiting for a DNS query to resolve.
	///@}

///@}

/// @defgroup ARPConfig ARP configuration
/// Size of ARP table and TTL
///
///@{

static const uint8_t ARP_TABLE_LENGTH = 2; //!< Maximum number of entries in the ARP table. Each ARP entry takes up `sizeof (ARPEntry) = 11` bytes, so size carefully.
static const int16_t MAX_ARP_TTL = 20; //!< Time the ARP entry is considered fresh, in minutes.

///@}

/// @defgroup DNSConfig DNS configuration
/// DNS timeouts, in milliseconds
///
///@{

#define DNS_TIMEOUT_QUERY_MS 5000 //!< How long to wait before giving up on a DNS query

///@}


/// @defgroup DHCPConfig DHCP configuration and timeouts
/// DHCP options and timeout configuration
///
///@{

/// <summary>Enables sending hostname to DHCP server. By default, only enabled when debugging.
/// When this option is enabled, EtherFlow identifies itself before the DHCP server. Most routers show a list
/// of connected hosts along with their names. Enabling this option will make your application to show up there.
/// </summary>
#define ENABLE_DHCP_HOSTNAME _DEBUG

static const char DHCP_HOSTNAME[] PROGMEM = "ARDUINO"; //!< host name sent to DHCP server.

	/// @defgroup DHCPTimings DHCP state timeout configuration
	/// DHCP timeouts
	/// These represent how much patience will EtherFlow have waiting in this state before
	/// thinking something is wrong and doing something about it.
	///@{

	#define DHCP_TIMEOUT_SELECTING_MS  1000 //!< (in milliseconds) DHCP timeout in the SELECTING DHCP phase (waiting for reply to DHCP DISCOVER)
	#define DHCP_TIMEOUT_REQUESTING_MS  5000 //!< (in milliseconds) DHCP timeout in the REQUESTING phase (waiting for reply to DHCP REQUEST)
	#define DHCP_TIMEOUT_BOUND_MS  1000 //!< (in milliseconds) set to 1 second so as to decrement the renewal timer

	static const uint8_t DHCP_MAX_ATTEMPTS = 4; //!< how many times to retry if address request is denied.

	static const uint16_t DHCP_DEFAULT_RENEWAL_TIMER_S = 7200; //!< Time in *seconds* to hold the IP lease, unless stated otherwise by the DHCP server. Default is 2h.

	///@}

///@}

/// @defgroup ICMPConfig ICMP configuration
/// %ICMP configuration section
///
/// See also \ref ENABLE_ICMP_RX_CHECKSUM
///@{

static const uint8_t ICMP_PING_DATA_LENGTH = 32; //!< size of data to send as part of an echo request

///@}

/// @defgroup HTTPServerConfig HTTPServer class configuration
/// HTTPServer class configuration section
///
/// Defines buffer space dedicated to temporarily hold queryString and one header name/value pair while the request is being received.
/// The buffer space for header name/value is shared with queryString, therefore it is recommended that the sum
/// of both HTTP_SERVER_HEADER_NAME_MAX_LENGTH and HTTP_SERVER_HEADER_VALUE_MAX_LENGTH + 1 equals HTTP_SERVER_QUERY_STRING_MAX_LENGTH
///@{
#define HTTP_SERVER_HEADER_NAME_MAX_LENGTH 20 //!< max buffer to hold a header name, e.g. "Content-Type"
#define HTTP_SERVER_HEADER_VALUE_MAX_LENGTH 20 //!< max buffer to hold a header value, e.g. "text/html"
#define HTTP_SERVER_QUERY_STRING_MAX_LENGTH 41 //!< max buffer to hold the entire query string, e.g. "/digitalRead?pin=14"
///@}

// ----------------- END OF CONFIGURATION ------------------------

// what follows are just computed values based on the above.

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/// @defgroup ENC28J60Config_computed ENC28J60 Computed memory boundaries
/// Don't change anything in this section, since it is all computed from the values entered above.
/// RXSTART_INIT must be zero. See Rev. B4 Silicon Errata point 5. 
///@{
#define RXSTART_INIT  0x0000  //!<  start of RX buffer. Due to hardware bugs, this must be zero.
#define RXSTOP_INIT (RXBUFFER_SIZE-1)  //!< end of RX buffer. Make sure RXSTOP_INIT is an odd number due to hardware bugs.

#define TX_STATUS_VECTOR_SIZE  15 //!< don't change this, it is how it is.

#define TXSTART_INIT  (RXSTOP_INIT + 1)  //!< start of TX buffer
#define TXSTOP_INIT  (TXSTART_INIT + TXBUFFER_SIZE - 1)  //!< end of TX buffer

/// <summary> don't change this, that's just how much RAM there is in your ENC28J60</summary>
#define ENC28J60_MEMSIZE  8192 

#define SHARED_BUFFER_INIT  (TXSTOP_INIT + 1 + TX_STATUS_VECTOR_SIZE) //!< start of the shared buffer area
#define SHARED_BUFFER_CAPACITY  (ENC28J60_MEMSIZE - SHARED_BUFFER_INIT) //!< resulting capacity of the shared buffer area.

#define TXSTART_INIT_DATA (TXSTART_INIT + 1) // skip 1 byte to make room for the control byte required by ENC28J60

static const uint16_t i2 = TXSTOP_INIT;
static const uint16_t i1 = SHARED_BUFFER_INIT;
///@}

#define NTICKS(ms) ((ms)/NETWORK_TIMER_RESOLUTION)

static const uint8_t SCK_TIMEOUT_SYN_SENT = NTICKS(SCK_TIMEOUT_SYN_SENT_MS); // (client) represents waiting for a matching connection request after having sent a connection request.
static const uint8_t SCK_TIMEOUT_SYN_RECEIVED = NTICKS(SCK_TIMEOUT_SYN_RECEIVED_MS); //(server) SYN has been received, SYN ACK was sent and now waiting for ACK.
static const uint8_t SCK_TIMEOUT_FIN_WAIT_1 = NTICKS(SCK_TIMEOUT_FIN_WAIT_1_MS); // (both server and client) represents waiting for a connection termination request from the remote TCP, or an acknowledgment of the connection termination request previously sent.
static const uint8_t SCK_TIMEOUT_FIN_WAIT_2 = NTICKS(SCK_TIMEOUT_FIN_WAIT_2_MS); // (both server and client) represents waiting for a connection termination request from the remote TCP.
static const uint8_t SCK_TIMEOUT_CLOSE_WAIT = NTICKS(SCK_TIMEOUT_CLOSE_WAIT_MS); // (both server and client) represents waiting for a connection termination request from the local user.
static const uint8_t SCK_TIMEOUT_CLOSING = NTICKS(SCK_TIMEOUT_CLOSING_MS); // (both server and client) represents waiting for a connection termination request acknowledgment from the remote TCP.
static const uint8_t SCK_TIMEOUT_LAST_ACK = NTICKS(SCK_TIMEOUT_LAST_ACK_MS); // (both server and client) represents waiting for an acknowledgment of the connection termination request previously sent to the remote TCP (which includes an acknowledgment of its connection termination request).
static const uint8_t SCK_TIMEOUT_TIME_WAIT = NTICKS(SCK_TIMEOUT_TIME_WAIT_MS); //(either server or client) represents waiting for enough time to pass to be sure the remote TCP received the acknowledgment of its connection termination request. [According to RFC 793 a connection can stay in TIME-WAIT for a maximum of four minutes known as a MSL (maximum segment lifetime).]
static const uint8_t SCK_TIMEOUT_RESOLVING = NTICKS(SCK_TIMEOUT_RESOLVING_MS); // waiting for a DNS query to resolve

static const uint8_t DNS_TIMEOUT_QUERY = NTICKS(DNS_TIMEOUT_QUERY_MS);//DNS timeout

static const uint8_t DHCP_TIMEOUT_SELECTING = NTICKS(DHCP_TIMEOUT_SELECTING_MS);
static const uint8_t DHCP_TIMEOUT_REQUESTING = NTICKS(DHCP_TIMEOUT_REQUESTING_MS);
static const uint8_t DHCP_TIMEOUT_BOUND = NTICKS(DHCP_TIMEOUT_BOUND_MS); 
static const uint16_t DHCP_DEFAULT_RENEWAL_TIMER = DHCP_DEFAULT_RENEWAL_TIMER_S * 1000 / DHCP_TIMEOUT_BOUND_MS;

#define QUOTE(str) #str

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

#define BUFFER_END (SHARED_BUFFER_INIT + SHARED_BUFFER_CAPACITY)

#if (BUFFER_END) != (ENC28J60_MEMSIZE)
#error Memory mapping error
#endif

#if RXBUFFER_SIZE & 1
#error RXBUFFER_SIZE must be an even number
#endif

#if (SHARED_BUFFER_CAPACITY) < 2048
#error Resulting shared buffer capacity is too small. Reduce TXBUFFER_SIZE or RXBUFFER_SIZE to allow for more shared buffer.
#endif

#endif

///@}

#endif