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


#include "Checksum.h"

/// <summary>
/// Adds two checksums, taking carry into account.
/// </summary>
/// <param name="a">the first checksum value</param>
/// <param name="b">The second checksum value to add</param>
/// <returns>the computed checksum</returns>
uint16_t Checksum::add(uint16_t a, uint16_t b)
{
	a += b;

	if (a<b)
		a++;

	return a;
}

/// <summary>
/// Adds two checksums, taking carry into account and whether the second one starts
/// at an odd index
/// </summary>
/// <param name="a">the first checksum value</param>
/// <param name="b">The second checksum value to add</param>
/// <param name="odd">Whether the second value (b) starts at an odd index</param>
/// <returns>the computed checksum</returns>
uint16_t Checksum::add(uint16_t a, uint16_t b, bool odd)
{
	if (odd)
		b = (b << 8) | (b >> 8);

	return add(a, b);

}


/// <summary>
/// Calculates the checksum of the specified buffer in memory
/// </summary>
/// <param name="len">length of the buffer</param>
/// <param name="data">The data.</param>
/// <returns>the computed checksum</returns>
uint16_t Checksum::calc(uint16_t len, const uint8_t *data)
{
	uint16_t sum = 0;
	const uint16_t *dataptr;
	const uint16_t *last;

	dataptr = (uint16_t*)data;
	last = dataptr + (len >> 1);

	while (dataptr < last)
	{
		sum = add(sum, *dataptr);
		dataptr++;
	}

	if (len & 1)
		sum = add(sum, *((uint8_t*)dataptr));

	return sum;

}

/// <summary>
/// Calculates the checksum of the specified buffer in memory, 
/// as a continuation of a previoulsy calculated checksum
/// </summary>
/// <param name="checksum">Previous checksum to add to</param>
/// <param name="len">length of the buffer</param>
/// <param name="data">The data.</param>
/// <returns>the computed checksum</returns>
uint16_t Checksum::calc(uint16_t checksum, uint16_t len, const uint8_t *data)
{
	return add(checksum, calc(len, data));
}
