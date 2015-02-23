#ifndef _EasyHTTPClient_H_
#define _EasyHTTPClient_H_

#include <ACross.h>
#include <FlowScanner.h>
#include "HTTPClient.h"

#define HEADER_NAME_MAX_LENGTH 30
#define HEADER_VALUE_MAX_LENGTH 30

#define QUOTE(str) #str

#define headerPatternStringMaxLength(headerLength, valueLength) "%" QUOTE(headerLength) "[^:]:%*9[ ]%" QUOTE(valueLength) "[^\r\n]\r\n"

const char headerPatternString[] PROGMEM = headerPatternStringMaxLength(HEADER_NAME_MAX_LENGTH, HEADER_VALUE_MAX_LENGTH);



class EasyHTTPClient : public HTTPClient
{

};




#endif
