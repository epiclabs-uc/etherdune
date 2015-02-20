#include <ACross.h>
#include "TCPSocket.h"
#include <FlowScanner.h>

const char statusCodePatternString[] PROGMEM = "HTTP/%*1d.%*1d %d%*99[^\r\n]\r\n";
const char bodyBeginPatternString[] PROGMEM = "\r\n\r\n";

class HTTPClient : public TCPSocket
{

private:
	void onConnect();
	void onReceive(uint16_t len, const byte* data);
	void onClose();
	
	FlowPattern statusCodePattern;
	FlowPattern bodyBeginPattern;
	FlowScanner scanner;

	String host;
	String res;



public:
	HTTPClient();
	~HTTPClient();
	uint32_t contentLength;
	uint16_t statusCode;

	void request(const String& hostName, const String& resource);

	virtual void onResponseReceived();
	virtual void onResponseEnd();
	virtual void onHeaderReceived(uint16_t len, const byte* data);
	virtual void onBodyReceived(uint16_t len, const byte* data);


};