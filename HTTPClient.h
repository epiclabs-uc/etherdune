#include <ACross.h>
#include "TCPSocket.h"


class HTTPClient : public TCPSocket
{

private:
	void onConnect();
	void onReceive(uint16_t len, const byte* data);
	String host;
	String res;

public:

	void request(const String& hostName, const String& resource);

	


};