# EtherDune - Easy and extensible Arduino TCP/IP library
by Javier Peletier - jm@friendev.com

Main page and documentation: http://friendev.com/EtherDune/

EtherDune is a new easy to use library for Arduino featuring:

* Event-driven, non-blocking asynchronous API
* Network services architecture:
  * Allows sharing hardware in a clean, non-interfering way.
  * Only the services you explicitly use in your code (e.g., DHCP, DNS, ICMP, TCP...) are compiled into your binary.
* Extensible: Derive from a class to write your own protocol, at any level, e.g.:
  * Derive from NetworkService to implement any Ethernet-level protocol.
  * Derive from TCPSocket to implement TCP-based protocols
  * Derive from HTTPClient to connect to a web server
  * And many more: UDPSocket, HTTPServer...
* Extensive use of PROGMEM and facilities to enable you to store IP addresses, MAC addresses in PROGMEM, as well as always accepting PROGMEM-stored strings.
* TCP and UDP support:
  * Buffered write, making extensive use of ENC28J60's RAM
  * Multiple simultaneous active sockets.
  * Built-in templated write, similar to printf.
* Out-of-the-box: TCP, UDP, DHCP, DNS, ICMP (ping), HTTP client, HTTP server
* ACross-compatible (debug with your computer)
* Logging/tracing support setting the _DEBUG to preprocessor variable to 1
* Open source - GPLv2
* Full documentation
* Detailed examples to get you started
 
Supported hardware:

* Microchip's ENC28J60
* Tested on Atmel 328p-based Arduinos, but should work on most.
 

How to install:

* Download dependency libraries:
  * FlowScanner - scanf-like state machine for extracting data out of streams
  * ACross - cross-compile toolkit and runtime
* Download EtherDune from GitHub
* Install by adding the contents of each .zip file to your Arduino libraries folder



Copyright (c) 2015 All Rights Reserved, http://friendev.com

This source is subject to the GPLv2 license.
Please see the License.txt file for more information.
All other rights reserved.

THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.