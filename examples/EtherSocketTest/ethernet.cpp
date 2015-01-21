

#include "ethernet.h"
#include <stdarg.h>
#include <avr/eeprom.h>




uint8_t selectPin;
static byte Enc28j60Bank;
static int gNextPacketPtr;

bool EtherSocket::broadcast_enabled = false;


MACAddress EtherSocket::localMAC;
IPAddress EtherSocket::localIP;

uint8_t EtherSocket::availableSlots = NUM_SLOTS;
uint16_t EtherSocket::availableSlotBitmap = 0;


ARPEntry arpTable[ARP_TABLE_LENGTH];
EthBuffer EtherSocket::chunk;
Socket* EtherSocket::sockets[MAX_TCP_SOCKETS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
Socket* EtherSocket::currentSocket = NULL;

static unsigned long tickTimer = NETWORK_TIMER_RESOLUTION;
static uint16_t minuteTimer = 60 * 1000 / NETWORK_TIMER_RESOLUTION;

void initSPI() {
	pinMode(SS, OUTPUT);
	digitalWrite(SS, HIGH);
	pinMode(MOSI, OUTPUT);
	pinMode(SCK, OUTPUT);
	pinMode(MISO, INPUT);

	digitalWrite(MOSI, HIGH);
	digitalWrite(MOSI, LOW);
	digitalWrite(SCK, LOW);

	SPCR = bit(SPE) | bit(MSTR); // 8 MHz @ 16
	bitSet(SPSR, SPI2X);
}

static void enableChip() {
	cli();
	digitalWrite(selectPin, LOW);
}

static void disableChip() {
	digitalWrite(selectPin, HIGH);
	sei();
}

static void xferSPI(byte data) {
	SPDR = data;
	while (!(SPSR&(1 << SPIF)))
		;
}

static byte readOp(byte op, byte address) {
	enableChip();
	xferSPI(op | (address & ADDR_MASK));
	xferSPI(0x00);
	if (address & 0x80)
		xferSPI(0x00);
	byte result = SPDR;
	disableChip();
	return result;
}



static void writeOp(byte op, byte address, byte data) {
	enableChip();
	xferSPI(op | (address & ADDR_MASK));
	xferSPI(data);
	disableChip();
}

static void SetBank(byte address) {
	if ((address & BANK_MASK) != Enc28j60Bank) {
		writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_BSEL1 | ECON1_BSEL0);
		Enc28j60Bank = address & BANK_MASK;
		writeOp(ENC28J60_BIT_FIELD_SET, ECON1, Enc28j60Bank >> 5);
	}
}

static byte readRegByte(byte address) {
	SetBank(address);
	return readOp(ENC28J60_READ_CTRL_REG, address);
}

static uint16_t readReg(byte address) {
	return readRegByte(address) + (readRegByte(address + 1) << 8);
}


static void writeRegByte(byte address, byte data) {
	SetBank(address);
	writeOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

static void writeReg(byte address, uint16_t data) {
	writeRegByte(address, data);
	writeRegByte(address + 1, data >> 8);
}

static void readBuf(uint16_t len, byte* data) {
	enableChip();
	xferSPI(ENC28J60_READ_BUF_MEM);
	while (len--) {
		xferSPI(0x00);
		*data++ = SPDR;
	}
	disableChip();
}

static void readBuf(uint16_t src, uint16_t len, byte* data)
{
	writeReg(ERDPT, src);
	readBuf(len, data);
}

static void writeBuf(uint16_t len, const byte* data) {
	enableChip();
	xferSPI(ENC28J60_WRITE_BUF_MEM);
	while (len--)
		xferSPI(*data++);
	disableChip();
}

static void writeBuf(uint16_t dst, uint16_t len, const byte* data)
{

	writeReg(EWRPT, dst);
	//writeOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	writeBuf(len, data);

	//writeReg(ERDPT, txStart);
}

static void writeByte(byte b)
{
	//writeOp(ENC28J60_WRITE_BUF_MEM, 0, b);
	writeBuf(1, &b);
}
static void writeByte(uint16_t dst, byte b)
{
	writeBuf(dst, 1, &b);
}



void writeSlot(uint8_t slot, uint16_t offset, const byte * data, uint16_t len)
{
	uint16_t txStart = SLOT_ADDR(slot);
	writeReg(EWRPT, txStart+offset);
	writeBuf(len, data);
	writeReg(ERDPT, txStart);

}

void packetSend(uint16_t len)
{
	// see http://forum.mysensors.org/topic/536/
	// while (readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS)
	if (readRegByte(EIR) & EIR_TXERIF) {
		writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
		writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
		writeOp(ENC28J60_BIT_FIELD_CLR, EIR, EIR_TXERIF);
	}

	writeByte(TXSTART_INIT, 0x00); // set the control byte to zero.
	
	writeReg(ETXST, TXSTART_INIT);
	writeReg(ETXND, TXSTART_INIT + len - 1 + 1); // to include the control byte.

	writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);

}

void packetSend(uint16_t len, const byte* data)
{
	writeBuf(TXSTART_INIT_DATA, len, data);
	packetSend(len);
}

void EtherSocket::enableBroadcast(bool temporary) {
	writeRegByte(ERXFCON, readRegByte(ERXFCON) | ERXFCON_BCEN);
	if (!temporary)
		broadcast_enabled = true;
}

static void writePhy(byte address, uint16_t data) {
	writeRegByte(MIREGADR, address);
	writeReg(MIWR, data);
	while (readRegByte(MISTAT) & MISTAT_BUSY)
		;
}

static uint16_t readPhyByte(byte address) {
	writeRegByte(MIREGADR, address);
	writeRegByte(MICMD, MICMD_MIIRD);
	while (readRegByte(MISTAT) & MISTAT_BUSY)
		;
	writeRegByte(MICMD, 0x00);
	return readRegByte(MIRD + 1);
}

bool EtherSocket::isLinkUp() {
	return (readPhyByte(PHSTAT2) >> 2) & 1;
}

uint8_t EtherSocket::begin(uint8_t cspin)
{
	memset(arpTable, -2, ARP_TABLE_LENGTH * sizeof(ARPEntry));
	//memset(sockets, 0, sizeof(Socket*) * MAX_TCP_SOCKETS);
	
	if (bitRead(SPCR, SPE) == 0)
		initSPI();
	selectPin = cspin;
	pinMode(selectPin, OUTPUT);
	disableChip();

	writeOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	delay(2); // errata B7/2
	while (!readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY)
		;

	gNextPacketPtr = RXSTART_INIT;
	writeReg(ERXST, RXSTART_INIT);
	writeReg(ERXRDPT, RXSTART_INIT);
	writeReg(ERXND, RXSTOP_INIT);
	writeReg(ETXST, TXSTART_INIT);
	writeReg(ETXND, TXSTOP_INIT);
	enableBroadcast(); // change to add ERXFCON_BCEN recommended by epam
	writeReg(EPMM0, 0x303f);
	writeReg(EPMCS, 0xf7f9);
	writeRegByte(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
	writeRegByte(MACON2, 0x00);
	writeOp(ENC28J60_BIT_FIELD_SET, MACON3,
		MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
	writeReg(MAIPG, 0x0C12);
	writeRegByte(MABBIPG, 0x12);
	writeReg(MAMXFL, MAX_FRAMELEN);
	writeRegByte(MAADR5, localMAC.b[0]);
	writeRegByte(MAADR4, localMAC.b[1]);
	writeRegByte(MAADR3, localMAC.b[2]);
	writeRegByte(MAADR2, localMAC.b[3]);
	writeRegByte(MAADR1, localMAC.b[4]);
	writeRegByte(MAADR0, localMAC.b[5]);
	writePhy(PHCON2, PHCON2_HDLDIS);
	SetBank(ECON1);
	writeOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
	writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

	byte rev = readRegByte(EREVID);
	// microchip forgot to step the number on the silcon when they
	// released the revision B7. 6 is now rev B7. We still have
	// to see what they do when they release B8. At the moment
	// there is no B8 out yet
	if (rev > 5) ++rev;
	return rev;



}

uint16_t EtherSocket::packetReceiveChunk()
{
	uint16_t len = 0;
	if (readRegByte(EPKTCNT) > 0)
	{
		writeReg(ERDPT, gNextPacketPtr);

		struct
		{
			uint16_t nextPacket;
			uint16_t byteCount;
			uint16_t status;
		} header;

		readBuf(sizeof header, (byte*)&header);

		gNextPacketPtr = header.nextPacket;
		len = header.byteCount - 4; //remove the CRC count

		if ((header.status & 0x80) == 0)
			len = 0;

		uint16_t chunkLength;

		uint8_t handler = 0;
		while (len > 0)
		{
			chunkLength = min(sizeof(chunk), len);
			readBuf(chunkLength, chunk.raw);
			processChunk(handler, chunkLength);
			len -= chunkLength;
		}
		currentSocket = NULL;

		if (gNextPacketPtr - 1 > RXSTOP_INIT)
			writeReg(ERXRDPT, RXSTOP_INIT);
		else
			writeReg(ERXRDPT, gNextPacketPtr - 1);
		writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	}
	return len;

}

void EtherSocket::loop()
{
	packetReceiveChunk();


	if ((long)(millis() - tickTimer) >= 0)
	{
		
		tick();
		tickTimer += NETWORK_TIMER_RESOLUTION;
	}

}

void EtherSocket::tick()
{

	for (int i = 0; i < MAX_TCP_SOCKETS; i++)
		if (sockets[i] != NULL)
			sockets[i]->tick();


	minuteTimer--;

	if (minuteTimer == 0)
	{
		minuteTimer = 60 * 1000 / NETWORK_TIMER_RESOLUTION;

		for (ARPEntry* entry = arpTable + (ARP_TABLE_LENGTH - 1); entry >= arpTable; entry--)
		{
			if (entry->status_TTL > 0)
				entry->status_TTL--;
		}
	}

}


void EtherSocket::processChunk(uint8_t& handler, uint16_t len)
{
	switch (handler)
	{
		case 0:
		{
			if (chunk.etherType.getValue() == ETHTYPE_ARP && chunk.arp.OPER.l == ARP_OPCODE_REPLY_L)
			{
				Serial.print("ARP Reply received=");
				Serial.println(chunk.arp.senderMAC.b[1]);
				processARPReply();
				return;
			}

			if (chunk.etherType.getValue() == ETHTYPE_IP && chunk.ip.protocol == IP_PROTO_TCP_V)
			{
				handler = 1;
				processTCPSegment(true, len);
			}
		}break;

		case 1:
		{
			processTCPSegment(false, len);
		}break;


		default:
		{
			handler = 0xFF; //discard further chunks of this packet.
		}
	}

}

void EtherSocket::processTCPSegment(bool isHeader, uint16_t len)
{
	if (isHeader)
	{
		for (uint8_t i = 0; i < MAX_TCP_SOCKETS; i++)
		{
			if (sockets[i]->srcPort_L == chunk.tcp.destinationPort.l && chunk.tcp.destinationPort.h == TCP_SRC_PORT_H)
				currentSocket = sockets[i];
		}
	}

	currentSocket->processSegment(isHeader);

}



MACAddress* EtherSocket::whoHas(IPAddress& ip)
{
	for (ARPEntry* entry = arpTable + (ARP_TABLE_LENGTH-1);entry >= arpTable;entry--)
	{
		if (ip.u == entry->ip.u && entry->status_TTL > 0)
		{
			entry->status_TTL = MAX_ARP_TTL;
			return &entry->mac;
		}
	}

	makeWhoHasARPRequest(ip);

	return NULL;


}



void EtherSocket::makeWhoHasARPRequest(IPAddress& ip)
{
	memset(&chunk.dstMAC, 0xFF, sizeof(MACAddress));
	chunk.srcMAC = chunk.arp.senderMAC = localMAC;
	chunk.etherType.setValue(ETHTYPE_ARP);
	chunk.arp.HTYPE.setValue(0x0001);
	chunk.arp.PTYPE.setValue(0x0800);
	chunk.arp.HLEN = 0x06;
	chunk.arp.PLEN = 0x04;
	chunk.arp.OPER.setValue(0x0001);
	memset(&chunk.arp.targetMAC, 0x00, sizeof(MACAddress));
	chunk.arp.targetIP = ip;
	chunk.arp.senderIP = localIP;

	

	packetSend(6 + 6 + 2 + 28, chunk.raw);

}

void EtherSocket::processARPReply()
{
	int16_t lowest = MAX_ARP_TTL;
	ARPEntry * selectedEntry;
	for (ARPEntry* entry = arpTable + (ARP_TABLE_LENGTH - 1); entry >= arpTable; entry--)
	{
		if (entry->status_TTL < lowest)
		{
			lowest = entry->status_TTL;
			selectedEntry = entry;
		}
	}

	selectedEntry->status_TTL = MAX_ARP_TTL;
	selectedEntry->ip = chunk.arp.senderIP;
	selectedEntry->mac = chunk.arp.senderMAC;

}

uint8_t EtherSocket::getSlot()
{

	uint16_t mask = 1;
	for (uint8_t i = 0; i < NUM_SLOTS;i++, mask <<= 1)
	{
		if (availableSlotBitmap & mask == 0)
		{
			availableSlots--;
			availableSlotBitmap |= mask;
			return i;
		}
	}

	return 0xFF; // no more slots available.
}

void EtherSocket::freeSlot(uint8_t slotId)
{
	uint16_t mask = 1 << slotId;

	if (availableSlotBitmap & mask)
	{
		availableSlots++;
		availableSlotBitmap &= ~mask;
	}
}

/// <summary>
/// Computes  a TCP/IP-type checksum over the specified buffer
/// </summary>
/// <param name="sum">previous checksum of another buffer attached to this one for checksum calculation purposes</param>
/// <param name="data">pointer to data to calculate checksum of</param>
/// <param name="len">lenghth of buffer</param>
/// <param name="carry">previous carry</param>
/// <param name="odd">Input: Whether the buffer starts at an odd index position, output: whether the next position will be odd</param>
/// <returns></returns>
uint16_t EtherSocket::checksum(uint16_t sum, const uint8_t *data, uint16_t len, bool &carry, bool& odd)
{
	uint16_t t;

	for (int i = 0; i < len; i++)
	{
		if (odd) // odd index
		{
			t = data[i];
		}
		else
		{
			t = ((uint16_t)data[i]) << 8;
			carry = false;
		}
		sum += t;
		if ((sum <t) && !carry)
		{
			sum++;
			carry = true;
		}

		odd = !odd;
	}

	return sum;
}

/// <summary>
/// Calculates the checksum of an even number of bytes that start at an even index position.
/// </summary>
/// <param name="sum">previous checksum of another buffer attached to this one for checksum calculation purposes</param>
/// <param name="data">pointer to data to calculate checksum of</param>
/// <param name="len">lenghth of buffer</param>
/// <returns></returns>
uint16_t EtherSocket::checksum(uint16_t sum, const uint8_t *data, uint16_t len)
{
	bool carry = false;
	bool odd = false;
	return checksum(sum, data, len, carry, odd);
}


void EtherSocket::sendIPPacket()
{
	IPAddress dstIP = chunk.ip.destinationIP;
	MACAddress* dstMac = whoHas(dstIP);

	if (dstMac == NULL)
		return;

	chunk.dstMAC = *dstMac;
	chunk.srcMAC = localMAC;
	chunk.etherType.setValue(ETHTYPE_IP);

	packetSend(6 + 6 + 2 + chunk.ip.totalLength.getValue(),chunk.raw);

}

void EtherSocket::registerSocket(Socket& socket)
{
	for (int i = 0; i < MAX_TCP_SOCKETS; i++)
	{
		if (sockets[i] == NULL)
		{
			sockets[i] = &socket;
			return;
		}
	}
}

void EtherSocket::unregisterSocket(Socket& socket)
{
	for (int i = 0; i < MAX_TCP_SOCKETS; i++)
	{
		if (sockets[i] == &socket)
		{
			sockets[i] = NULL;
			return;
		}
	}

}