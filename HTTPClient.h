#include <ACross.h>
#include "TCPSocket.h"
#include <FlowScanner.h>


class HTTPClient : public TCPSocket
{

private:
	void onConnect();
	void onReceive(uint16_t len, const byte* data);
	void onClose();
	
	FlowScanner headerScanner;
	FlowScanner bodyBeginScanner;

	String host;
	String res;
	char headerName[30];
	char headerValue[30];


public:
	HTTPClient();
	~HTTPClient();
	uint32_t contentLength;
	uint16_t statusCode;

	void request(const String& hostName, const String& resource);

	virtual void onHeaderReceived(const char* header, const char* value);
	virtual void onBodyReceived(uint16_t len, const byte* data);


};