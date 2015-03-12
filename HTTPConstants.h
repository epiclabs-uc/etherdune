// EtherFlow HTTP Constants
// Author: Javier Peletier <jm@friendev.com>
// Summary: All constants used in all HTTP-related classes in EtherFlow
// Credits: Built from sources across the Internet, Wikipedia, Mozilla.org, etc
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

#ifndef _HTTP_CONSTANTS_
#define _HTTP_CONSTANTS_
#include <ACross.h>

/// @defgroup HTTPConstants HTTP Protocol constants
/// @details All constants used in all HTTP-related classes in EtherFlow
/// @{

/// @defgroup HTTPConstants_ContentType Content-Type constants
/// @details Common content-type strings, defined as PROGMEM
/// @{


static const char CONTENT_TYPE_TEXT_HTML[] PROGMEM = "text/html";
static const char CONTENT_TYPE_APPLICATION_JSON[] PROGMEM = "application/json";

/// @}

/// @defgroup HTTPConstants_Methods HTTP method constants
/// @details %List of all possible HTTP methods
/// @{

static const uint8_t HTTP_METHOD_OPTIONS = 44;
static const uint8_t HTTP_METHOD_GET = 230;
static const uint8_t HTTP_METHOD_HEAD = 26;
static const uint8_t HTTP_METHOD_POST = 78;
static const uint8_t HTTP_METHOD_PUT = 255;
static const uint8_t HTTP_METHOD_DELETE = 191;
static const uint8_t HTTP_METHOD_TRACE = 121;
static const uint8_t HTTP_METHOD_CONNECT = 24;
static const uint8_t HTTP_METHOD_PROPFIND = 114;
static const uint8_t HTTP_METHOD_PROPPATCH = 195;
static const uint8_t HTTP_METHOD_MKCOL = 128;
static const uint8_t HTTP_METHOD_COPY = 67;
static const uint8_t HTTP_METHOD_MOVE = 63;
static const uint8_t HTTP_METHOD_LOCK = 49;
static const uint8_t HTTP_METHOD_UNLOCK = 216;
static const uint8_t HTTP_METHOD_VERSION_CONTROL = 146;
static const uint8_t HTTP_METHOD_REPORT = 232;
static const uint8_t HTTP_METHOD_CHECKOUT = 102;
static const uint8_t HTTP_METHOD_CHECKIN = 3;
static const uint8_t HTTP_METHOD_UNCHECKOUT = 13;
static const uint8_t HTTP_METHOD_MKWORKSPACE = 93;
static const uint8_t HTTP_METHOD_UPDATE = 207;
static const uint8_t HTTP_METHOD_LABEL = 106;
static const uint8_t HTTP_METHOD_MERGE = 122;
static const uint8_t HTTP_METHOD_BASELINE_CONTROL = 177;
static const uint8_t HTTP_METHOD_MKACTIVITY = 25;
static const uint8_t HTTP_METHOD_ORDERPATCH = 0;
static const uint8_t HTTP_METHOD_ACL = 214;
static const uint8_t HTTP_METHOD_SEARCH = 194;

/// @}

/// @defgroup HTTPConstants_ResponseCodes HTTP response codes
/// @details %List of HTTP response codes
/// @{

static const uint16_t HTTP_RESPONSE_CONTINUE = 100; //!< This interim response indicates that everything so far is OK and that the client should continue with the request or ignore it if it is already finished.
static const uint16_t HTTP_RESPONSE_SWITCHING_PROTOCOL = 101; //!< This code is sent in response to an Upgrade: request header by the client, and indicates that the protocol the server is switching too. It was introduced to allow migration to an incompatible protocol version, and is not in common use.
static const uint16_t HTTP_RESPONSE_OK = 200; //!< The request has succeeded. The meaning of a success varies depending on the HTTP method .
static const uint16_t HTTP_RESPONSE_CREATED = 201; //!< The request has succeeded and a new resource has been created as a result of it. This is typically the response sent after a PUT request.
static const uint16_t HTTP_RESPONSE_ACCEPTED = 202; //!< The request has been received but not yet acted upon. It is non-committal, meaning that there is no way in HTTP to later send an asynchronous response indicating the outcome of processing the request. It is intended for cases where another process or server handles the request, or for batch processing.
static const uint16_t HTTP_RESPONSE_NON_AUTHORITATIVE_INFORMATION = 203; //!< This response code means returned meta-information set is not exact set as available from the origin server, but collected from a local or a third party copy. Except this condition, 200 OK response should be preferred instead of this response.
static const uint16_t HTTP_RESPONSE_NO_CONTENT = 204; //!< There is no content to send for this request, but the headers may be useful. The user-agent may update its cached headers for this resource with the new ones.
static const uint16_t HTTP_RESPONSE_RESET_CONTENT = 205; //!< This response code is sent after accomplishing request to tell user agent reset document view which sent this request.
static const uint16_t HTTP_RESPONSE_PARTIAL_CONTENT = 206; //!< This response code is used because of range header sent by the client to separate download into multiple streams.
static const uint16_t HTTP_RESPONSE_MULTIPLE_CHOICE = 300; //!< The request has more than one possible responses. User-agent or user should choose one of them. There is no standardized way to choose one of the responses.
static const uint16_t HTTP_RESPONSE_MOVED_PERMANENTLY = 301; //!< This response code means that URI of requested resource has been changed. Probably, new URI would be given in the response.
static const uint16_t HTTP_RESPONSE_FOUND = 302; //!< This response code means that URI of requested resource has been changed temporarily. New changes in the URI might be made in the future. Therefore, this same URI should be used by the client in future requests.
static const uint16_t HTTP_RESPONSE_SEE_OTHER = 303; //!< Server sent this response to directing client to get requested resource to another URI with an GET request.
static const uint16_t HTTP_RESPONSE_NOT_MODIFIED = 304; //!< This is used for caching purposes. It is telling to client that response has not been modified. So, client can continue to use same cached version of response.
static const uint16_t HTTP_RESPONSE_USE_PROXY = 305; //!< This means requested response must be accessed by a proxy. This response code is not largely supported because security reasons.
static const uint16_t HTTP_RESPONSE_UNUSED = 306; //!< This response code is no longer used, it is just reserved currently. It was used in a previous version of the HTTP 1.1 specification.
static const uint16_t HTTP_RESPONSE_TEMPORARY_REDIRECT = 307; //!< Server sent this response to directing client to get requested resource to another URI with same method that used prior request. This has the same semantic than the 302 Found HTTP response code, with the exception that the user agent must not change the HTTP method used: if a POSTwas used in the first request, a POST must be used in the second request.
static const uint16_t HTTP_RESPONSE_PERMANENT_REDIRECT = 308; //!< This means that the resource is now permanently located at another URI, specified by the Location: HTTP Response header. This has the same semantics as the 301 Moved Permanently HTTP response code, with the exception that the user agent must not change the HTTP method used: if a POST was used in the first request, a POST must be used in the second request.
static const uint16_t HTTP_RESPONSE_BAD_REQUEST = 400; //!< This response means that server could not understand the request due to invalid syntax.
static const uint16_t HTTP_RESPONSE_UNAUTHORIZED = 401; //!< Authentication is needed to get requested response. This is similar to 403, but in this case, authentication is possible.
static const uint16_t HTTP_RESPONSE_PAYMENT_REQUIRED = 402; //!< This response code is reserved for future use. Initial aim for creating this code was using it for digital payment systems however this is not used currently.
static const uint16_t HTTP_RESPONSE_FORBIDDEN = 403; //!< Client does not have access rights to the content so server is rejecting to give proper response.
static const uint16_t HTTP_RESPONSE_NOT_FOUND = 404; //!< Server can not find requested resource. This response code probably is most famous one due to its frequency to occur in web.
static const uint16_t HTTP_RESPONSE_METHOD_NOT_ALLOWED = 405; //!< The request method is known by the server but has been disabled and cannot be used. The two mandatory methods, GET and HEAD, must never be disabled and should not return this error code.
static const uint16_t HTTP_RESPONSE_NOT_ACCEPTABLE = 406; //!< This response is sent when the web server, after performing server-driven content negotiation, doesn't find any content following the criteria given by the user agent.
static const uint16_t HTTP_RESPONSE_PROXY_AUTHENTICATION_REQUIRED = 407; //!< This is similar to 401 but authentication is needed to be done by a proxy.
static const uint16_t HTTP_RESPONSE_REQUEST_TIMEOUT = 408; //!< This response is sent on an idle connection by some servers, even without any previous request by the client. It means that the server would like to shut down this unused connection. This response is used much more since some browsers, like Chrome or IE9, use HTTP preconnection mechanisms to speed up surfing (see bug 634278, which tracks the future implementation of such a mechanism in Firefox). Also note that some servers merely shut down the connection without sending this message.
static const uint16_t HTTP_RESPONSE_CONFLICT = 409; //!< This response would be sent when a request conflict with current state of server.
static const uint16_t HTTP_RESPONSE_GONE = 410; //!< This response would be sent when requested content has been deleted from server.
static const uint16_t HTTP_RESPONSE_LENGTH_REQUIRED = 411; //!< Server rejected the request because the Content-Length header field is not defined and the server requires it.
static const uint16_t HTTP_RESPONSE_PRECONDITION_FAILED = 412; //!< The client has indicated preconditions in its headers which the server does not meet.
static const uint16_t HTTP_RESPONSE_REQUEST_ENTITY_TOO_LARGE = 413; //!< Request entity is larger than limits defined by server; the server might close the connection or return an Retry-After header field.
static const uint16_t HTTP_RESPONSE_REQUEST_URI_TOO_LONG = 414; //!< The URI requested by the client is too long for the server to handle.
static const uint16_t HTTP_RESPONSE_UNSUPPORTED_MEDIA_TYPE = 415; //!< The media format of the requested data is not supported by the server, so the server is rejecting the request.
static const uint16_t HTTP_RESPONSE_REQUESTED_RANGE_NOT_SATISFIABLE = 416; //!< The range specified by the Range header field in the request can't be fulfilled; it's possible that the range is outside the size of the target URI's data.
static const uint16_t HTTP_RESPONSE_EXPECTATION_FAILED = 417; //!< This response code means the expectation indicated by the Expect request header field can't be met by the server.
static const uint16_t HTTP_RESPONSE_INTERNAL_SERVER_ERROR = 500; //!< The server has encountered a situation it doesn't know how to handle.
static const uint16_t HTTP_RESPONSE_NOT_IMPLEMENTED = 501; //!< The request method is not supported by the server and cannot be handled. The only methods that servers are required to support (and therefore that must not return this code) are GET and HEAD.
static const uint16_t HTTP_RESPONSE_BAD_GATEWAY = 502; //!< This error response means that the server, while working as a gateway to get a response needed to handle the request, got an invalid response.
static const uint16_t HTTP_RESPONSE_SERVICE_UNAVAILABLE = 503; //!< The server is not ready to handle the request. Common causes are a server that is down for maintenance or that is overloaded. Note that together with this response, a user-friendly page explaining the problem should be sent. This responses should be used for temporary conditions and theRetry-After: HTTP header should, if possible, contain the estimated time before the recovery of the service. The webmaster must also take care about the caching-related headers that are sent along with this response, as these temporary condition responses should usually not be cached.
static const uint16_t HTTP_RESPONSE_GATEWAY_TIMEOUT = 504; //!< This error response is given when the server is acting as a gateway and cannot get a response in time.
static const uint16_t HTTP_RESPONSE_HTTP_VERSION_NOT_SUPPORTED = 505; //!< The HTTP version used in the request is not supported by the server.

/// @}

/// @defgroup HTTPConstants_ResponseStrings typical HTTP response strings
/// @details %List of HTTP typical response strings
/// @{

static const char HTTP_RESPONSE_CONTINUE_STR[] PROGMEM = "Continue";
static const char HTTP_RESPONSE_SWITCHING_PROTOCOL_STR[] PROGMEM = "Switching Protocol";
static const char HTTP_RESPONSE_OK_STR[] PROGMEM = "OK";
static const char HTTP_RESPONSE_CREATED_STR[] PROGMEM = "Created";
static const char HTTP_RESPONSE_ACCEPTED_STR[] PROGMEM = "Accepted";
static const char HTTP_RESPONSE_NON_AUTHORITATIVE_INFORMATION_STR[] PROGMEM = "Non-Authoritative Information";
static const char HTTP_RESPONSE_NO_CONTENT_STR[] PROGMEM = "No Content";
static const char HTTP_RESPONSE_RESET_CONTENT_STR[] PROGMEM = "Reset Content";
static const char HTTP_RESPONSE_PARTIAL_CONTENT_STR[] PROGMEM = "Partial Content";
static const char HTTP_RESPONSE_MULTIPLE_CHOICE_STR[] PROGMEM = "Multiple Choice";
static const char HTTP_RESPONSE_MOVED_PERMANENTLY_STR[] PROGMEM = "Moved Permanently";
static const char HTTP_RESPONSE_FOUND_STR[] PROGMEM = "Found";
static const char HTTP_RESPONSE_SEE_OTHER_STR[] PROGMEM = "See Other";
static const char HTTP_RESPONSE_NOT_MODIFIED_STR[] PROGMEM = "Not Modified";
static const char HTTP_RESPONSE_USE_PROXY_STR[] PROGMEM = "Use Proxy";
static const char HTTP_RESPONSE_UNUSED_STR[] PROGMEM = "unused";
static const char HTTP_RESPONSE_TEMPORARY_REDIRECT_STR[] PROGMEM = "Temporary Redirect";
static const char HTTP_RESPONSE_PERMANENT_REDIRECT_STR[] PROGMEM = "Permanent Redirect";
static const char HTTP_RESPONSE_BAD_REQUEST_STR[] PROGMEM = "Bad Request";
static const char HTTP_RESPONSE_UNAUTHORIZED_STR[] PROGMEM = "Unauthorized";
static const char HTTP_RESPONSE_PAYMENT_REQUIRED_STR[] PROGMEM = "Payment Required";
static const char HTTP_RESPONSE_FORBIDDEN_STR[] PROGMEM = "Forbidden";
static const char HTTP_RESPONSE_NOT_FOUND_STR[] PROGMEM = "Not Found";
static const char HTTP_RESPONSE_METHOD_NOT_ALLOWED_STR[] PROGMEM = "Method Not Allowed";
static const char HTTP_RESPONSE_NOT_ACCEPTABLE_STR[] PROGMEM = "Not Acceptable";
static const char HTTP_RESPONSE_PROXY_AUTHENTICATION_REQUIRED_STR[] PROGMEM = "Proxy Authentication Required";
static const char HTTP_RESPONSE_REQUEST_TIMEOUT_STR[] PROGMEM = "Request Timeout";
static const char HTTP_RESPONSE_CONFLICT_STR[] PROGMEM = "Conflict";
static const char HTTP_RESPONSE_GONE_STR[] PROGMEM = "Gone";
static const char HTTP_RESPONSE_LENGTH_REQUIRED_STR[] PROGMEM = "Length Required";
static const char HTTP_RESPONSE_PRECONDITION_FAILED_STR[] PROGMEM = "Precondition Failed";
static const char HTTP_RESPONSE_REQUEST_ENTITY_TOO_LARGE_STR[] PROGMEM = "Request Entity Too Large";
static const char HTTP_RESPONSE_REQUEST_URI_TOO_LONG_STR[] PROGMEM = "Request-URI Too Long";
static const char HTTP_RESPONSE_UNSUPPORTED_MEDIA_TYPE_STR[] PROGMEM = "Unsupported Media Type";
static const char HTTP_RESPONSE_REQUESTED_RANGE_NOT_SATISFIABLE_STR[] PROGMEM = "Requested Range Not Satisfiable";
static const char HTTP_RESPONSE_EXPECTATION_FAILED_STR[] PROGMEM = "Expectation Failed";
static const char HTTP_RESPONSE_INTERNAL_SERVER_ERROR_STR[] PROGMEM = "Internal Server Error";
static const char HTTP_RESPONSE_NOT_IMPLEMENTED_STR[] PROGMEM = "Not Implemented";
static const char HTTP_RESPONSE_BAD_GATEWAY_STR[] PROGMEM = "Bad Gateway";
static const char HTTP_RESPONSE_SERVICE_UNAVAILABLE_STR[] PROGMEM = "Service Unavailable";
static const char HTTP_RESPONSE_GATEWAY_TIMEOUT_STR[] PROGMEM = "Gateway Timeout";
static const char HTTP_RESPONSE_HTTP_VERSION_NOT_SUPPORTED_STR[] PROGMEM = "HTTP Version Not Supported";

/// @}

/// @defgroup HTTPConstants_Headers HTTP headers
/// @details %List of common HTTP headers
/// @{

static const char HTTP_HEADER_ACCEPT[] PROGMEM = "Accept";  //!<  Content-Types that are acceptable for the response. See Content negotiation.
static const char HTTP_HEADER_ACCEPT_CHARSET[] PROGMEM = "Accept-Charset";  //!<  Character sets that are acceptable .
static const char HTTP_HEADER_ACCEPT_ENCODING[] PROGMEM = "Accept-Encoding";  //!<  %List of acceptable encodings. See HTTP compression.
static const char HTTP_HEADER_ACCEPT_LANGUAGE[] PROGMEM = "Accept-Language";  //!<  %List of acceptable human languages for response. See Content negotiation.
static const char HTTP_HEADER_ACCEPT_DATETIME[] PROGMEM = "Accept-Datetime";  //!<  Acceptable version in time .
static const char HTTP_HEADER_AUTHORIZATION[] PROGMEM = "Authorization";  //!<  Authentication credentials for HTTP authentication .
static const char HTTP_HEADER_CACHE_CONTROL[] PROGMEM = "Cache-Control";  //!<  Used to specify directives that must be obeyed by all caching mechanisms along the request-response chain .
static const char HTTP_HEADER_CONNECTION[] PROGMEM = "Connection";  //!<  Control options for the current connection and %List of hop-by-hop request fields[8] .
static const char HTTP_HEADER_COOKIE[] PROGMEM = "Cookie";  //!<  An HTTP cookie previously sent by the server with Set-Cookie (below) .
static const char HTTP_HEADER_CONTENT_LENGTH[] PROGMEM = "Content-Length";  //!<  The length of the request body in octets (8-bit bytes) .
static const char HTTP_HEADER_CONTENT_MD5[] PROGMEM = "Content-MD5";  //!<  A Base64-encoded binary MD5 sum of the content of the request body .
static const char HTTP_HEADER_CONTENT_TYPE[] PROGMEM = "Content-Type";  //!<  The MIME type of the body of the request (used with POST and PUT requests) .
static const char HTTP_HEADER_DATE[] PROGMEM = "Date";  //!<  The date and time that the message was sent (in "HTTP-date" format as defined by RFC 7231 Date/Time Formats) .
static const char HTTP_HEADER_EXPECT[] PROGMEM = "Expect";  //!<  Indicates that particular server behaviors are required by the client .
static const char HTTP_HEADER_FROM[] PROGMEM = "From";  //!<  The email address of the user making the request .
static const char HTTP_HEADER_HOST[] PROGMEM = "Host";  //!<  The domain name of the server (for virtual hosting), and the TCP port number on which the server is %Listening. The port number may be omitted if the port is the standard port for the service requested.
static const char HTTP_HEADER_IF_MATCH[] PROGMEM = "If-Match";  //!<  Only perform the action if the client supplied entity matches the same entity on the server. This is mainly for methods like PUT to only update a resource if it has not been modified since the user last updated it.
static const char HTTP_HEADER_IF_MODIFIED_SINCE[] PROGMEM = "If-Modified-Since";  //!<  Allows a 304 Not Modified to be returned if content is unchanged .
static const char HTTP_HEADER_IF_NONE_MATCH[] PROGMEM = "If-None-Match";  //!<  Allows a 304 Not Modified to be returned if content is unchanged, see HTTP ETag .
static const char HTTP_HEADER_IF_RANGE[] PROGMEM = "If-Range";  //!<  If the entity is unchanged, send me the part(s) that I am missing; otherwise, send me the entire new entity .
static const char HTTP_HEADER_IF_UNMODIFIED_SINCE[] PROGMEM = "If-Unmodified-Since";  //!<  Only send the response if the entity has not been modified since a specific time.
static const char HTTP_HEADER_MAX_FORWARDS[] PROGMEM = "Max-Forwards";  //!<  Limit the number of times the message can be forwarded through proxies or gateways.
static const char HTTP_HEADER_ORIGIN[] PROGMEM = "Origin";  //!<  Initiates a request for cross-origin resource sharing (asks server for an 'Access-Control-Allow-Origin' response field) .
static const char HTTP_HEADER_PRAGMA[] PROGMEM = "Pragma";  //!<  Implementation-specific fields that may have various effects anywhere along the request-response chain.
static const char HTTP_HEADER_PROXY_AUTHORIZATION[] PROGMEM = "Proxy-Authorization";  //!<  Authorization credentials for connecting to a proxy.
static const char HTTP_HEADER_RANGE[] PROGMEM = "Range";  //!<  Request only part of an entity. Bytes are numbered from 0. See Byte serving.
static const char HTTP_HEADER_REFERER[] PROGMEM = "Referer";  //!<  This is the address of the previous web page from which a link to the currently requested page was followed. (The word “referrer” has been misspelled in the RFC as well as in most implementations to the point that it has become standard usage and is considered correct terminology) .
static const char HTTP_HEADER_TE[] PROGMEM = "TE";  //!<  The transfer encodings the user agent is willing to accept: the same values as for the response header field Transfer-Encoding can be used, plus the "trailers" value (related to the "chunked" transfer method) to notify the server it expects to receive additional fields in the trailer after the last, zero-sized, packet.
static const char HTTP_HEADER_USER_AGENT[] PROGMEM = "User-Agent";  //!<  The user agent string of the user agent .
static const char HTTP_HEADER_UPGRADE[] PROGMEM = "Upgrade";  //!<  Ask the server to upgrade to another protocol.
static const char HTTP_HEADER_VIA[] PROGMEM = "Via";  //!<  Informs the server of proxies through which the request was sent.
static const char HTTP_HEADER_WARNING[] PROGMEM = "Warning";  //!<  A general warning about possible problems with the entity body.
static const char HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN[] PROGMEM = "Access-Control-Allow-Origin";  //!<  Specifying which web sites can participate in cross-origin resource sharing .
static const char HTTP_HEADER_ACCEPT_PATCH[] PROGMEM = "Accept-Patch";  //!<  Specifies which patch document formats this server supports .
static const char HTTP_HEADER_ACCEPT_RANGES[] PROGMEM = "Accept-Ranges";  //!<  What partial content range types this server supports .
static const char HTTP_HEADER_AGE[] PROGMEM = "Age";  //!<  The age the object has been in a proxy cache in seconds .
static const char HTTP_HEADER_ALLOW[] PROGMEM = "Allow";  //!<  Valid actions for a specified resource. To be used for a 405 Method not allowed .
static const char HTTP_HEADER_CONTENT_ENCODING[] PROGMEM = "Content-Encoding";  //!<  The type of encoding used on the data. See HTTP compression.
static const char HTTP_HEADER_CONTENT_LANGUAGE[] PROGMEM = "Content-Language";  //!<  The language the content is in .
static const char HTTP_HEADER_CONTENT_LOCATION[] PROGMEM = "Content-Location";  //!<  An alternate location for the returned data .
static const char HTTP_HEADER_CONTENT_DISPOSITION[] PROGMEM = "Content-Disposition";  //!<  An opportunity to raise a "File Download" dialogue box for a known MIME type with binary format or suggest a filename for dynamic content. Quotes are necessary with special characters.
static const char HTTP_HEADER_CONTENT_RANGE[] PROGMEM = "Content-Range";  //!<  Where in a full body message this partial message belongs .
static const char HTTP_HEADER_ETAG[] PROGMEM = "ETag";  //!<  An identifier for a specific version of a resource, often a message digest .
static const char HTTP_HEADER_EXPIRES[] PROGMEM = "Expires";  //!<  Gives the date/time after which the response is considered stale (in "HTTP-date" format as defined by RFC 7231) .
static const char HTTP_HEADER_LAST_MODIFIED[] PROGMEM = "Last-Modified";  //!<  The last modified date for the requested object (in "HTTP-date" format as defined by RFC 7231) .
static const char HTTP_HEADER_LINK[] PROGMEM = "Link";  //!<  Used to express a typed relationship with another resource, where the relation type is defined by RFC 5988 .
static const char HTTP_HEADER_LOCATION[] PROGMEM = "Location";  //!<  Used in redirection, or when a new resource has been created.
static const char HTTP_HEADER_P3P[] PROGMEM = "P3P";  //!<  This field is supposed to set P3P policy, in the form of P3P:CP="your_compact_policy". However, P3P did not take off,[31] most browsers have never fully implemented it, a lot of websites set this field with fake policy text, that was enough to fool browsers the existence of P3P policy and grant permissions for third party cookies.
static const char HTTP_HEADER_PROXY_AUTHENTICATE[] PROGMEM = "Proxy-Authenticate";  //!<  Request authentication to access the proxy.
static const char HTTP_HEADER_REFRESH[] PROGMEM = "Refresh";  //!<  Used in redirection, or when a new resource has been created. This refresh redirects after 5 seconds.
static const char HTTP_HEADER_RETRY_AFTER[] PROGMEM = "Retry-After";  //!<  If an entity is temporarily unavailable, this instructs the client to try again later. Value could be a specified period of time (in seconds) or a HTTP-date.[32] .
static const char HTTP_HEADER_SERVER[] PROGMEM = "Server";  //!<  A name for the server .
static const char HTTP_HEADER_SET_COOKIE[] PROGMEM = "Set-Cookie";  //!<  An HTTP cookie .
static const char HTTP_HEADER_STATUS[] PROGMEM = "Status";  //!<  CGI header field specifying the status of the HTTP response. Normal HTTP responses use a separate "Status-Line" instead, defined by RFC 7230.[33] .
static const char HTTP_HEADER_STRICT_TRANSPORT_SECURITY[] PROGMEM = "Strict-Transport-Security";  //!<  A HSTS Policy informing the HTTP client how long to cache the HTTPS only policy and whether this applies to subdomains.
static const char HTTP_HEADER_TRAILER[] PROGMEM = "Trailer";  //!<  The Trailer general field value indicates that the given set of header fields is present in the trailer of a message encoded with chunked transfer coding.
static const char HTTP_HEADER_TRANSFER_ENCODING[] PROGMEM = "Transfer-Encoding";  //!<  The form of encoding used to safely transfer the entity to the user. Currently defined methods are: chunked, compress, deflate, gzip, identity.
static const char HTTP_HEADER_VARY[] PROGMEM = "Vary";  //!<  Tells downstream proxies how to match future request headers to decide whether the cached response can be used rather than requesting a fresh one from the origin server.
static const char HTTP_HEADER_WWW_AUTHENTICATE[] PROGMEM = "WWW-Authenticate";  //!<  Indicates the authentication scheme that should be used to access the requested entity.

/// @}
/// @}

#endif