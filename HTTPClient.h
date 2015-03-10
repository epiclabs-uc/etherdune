#include <ACross.h>
#include "TCPSocket.h"
#include <FlowScanner.h>

static const char statusCodePatternString[] PROGMEM = "HTTP/%*1d.%*1d %d%*99[^\r\n]\r\n";
static const char bodyBeginPatternString[] PROGMEM = "\r\n\r\n";
static const char contentLengthPatternString[] PROGMEM = "Content-Length% :% %wd";

class HTTPClient : public TCPSocket
{

private:

	FlowPattern statusCodePattern;
	FlowPattern bodyBeginPattern;
	String host;
	String res;
	uint16_t DNSid;

	void onConnect();
	void onReceive(uint16_t len, const byte* data);
	void onClose();
	void onDNSResolve(uint8_t status, uint16_t identification, const IPAddress& ip);
	
protected:

	FlowScanner scanner;

public:

	uint16_t statusCode;

	HTTPClient();
	~HTTPClient();

	void request(const String& hostName, const String& resource, uint16_t port=80);

	virtual void onResponseReceived();
	virtual void onHeaderReceived(uint16_t len, const byte* data);
	virtual void onBodyReceived(uint16_t len, const byte* data);
	virtual void onBodyBegin();
	virtual void onResponseEnd();

};

