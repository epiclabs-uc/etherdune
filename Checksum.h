// EtherDune IP Checksum module
// Author: Javier Peletier <jm@friendev.com>
// Summary: memory-efficient algorithm to calculate total and partial network checksums
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

#ifndef _CHECKSUM_h
#define _CHECKSUM_h

#include <ACross.h>


class Checksum
{
public:
	static uint16_t add(uint16_t a, uint16_t b);
	static uint16_t add(uint16_t a, uint16_t b, bool odd);
	static uint16_t calc(uint16_t len, const uint8_t *data);
	static uint16_t calc(uint16_t checksum, uint16_t len, const uint8_t *data);

};


#endif

