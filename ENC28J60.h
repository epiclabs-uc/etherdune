#ifndef __EENC28J60__H_
#define __EENC28J60__H_

#define __PROG_TYPES_COMPAT__

#include <ACross.h>

#include "inet.h"
#include "enc28j60constants.h"
#include "config.h"


class NetworkService;
class SharedBuffer;

class ENC28J60 
{
	friend class SharedBuffer;

protected:

	static void writeByte(byte b);
	static void writeByte(uint16_t dst, byte b);
	static void writeBuf(uint16_t dst, uint16_t len, const byte* data);
	static void writeBuf(uint16_t len, const byte* data);
	static uint16_t hardwareChecksum(uint16_t src, uint16_t len);
	static uint16_t hardwareChecksumRxOffset(uint16_t offset, uint16_t len);
	static void moveMem(uint16_t dest, uint16_t src, uint16_t len);
	static void readBuf(uint16_t src, uint16_t len, byte* data);
	static void readBuf(uint16_t len, byte* data);
	static byte readByte(uint16_t src);
	static void packetSend(uint16_t len);
	static void packetSend(uint16_t len, const byte* data);
	static void loadSample();
	static void loadAll();
	static void release();
	static uint8_t begin(uint8_t cspin);
	static void loop();
	static void enableBroadcast(bool temporary = false);

public:

	static bool broadcast_enabled; //!< True if broadcasts enabled (used to allow temporary disable of broadcast for DHCP or other internal functions)

	static bool isLinkUp();
	
};

#endif
