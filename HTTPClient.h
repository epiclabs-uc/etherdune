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
	char* headerName;
	char* headerValue;


public:
	HTTPClient();
	~HTTPClient();
	uint32_t contentLength;
	uint16_t statusCode;

	void request(const String& hostName, const String& resource);

	virtual void onHeaderReceived(char* header, char* value);
	virtual void onBodyReceived(uint16_t len, const byte* data);


};