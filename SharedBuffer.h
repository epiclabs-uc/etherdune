// EtherFlow Shared circular buffer class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Implements a "shared" circular buffer using spare ENC28J60 memory
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

/**
\class SharedBuffer
\brief Implements a "shared" circular buffer using spare ENC28J60 memory
\details 
This class takes the memory that is not used for the receive/transmit buffer
and turns it into a circular buffer shared by all sockets or any network
service that needs to assemble packets before they are put on the network,
or in the case of TCP, buffer data in case a retransmit is necessary.

The concept of "shared circular buffer" is an attempt to provide each
network service with its own circular buffer but without having to
statically assign a chunk of dedicated memory to each possible service.
Instead, all services share the same memory chunk, which is about 4kB in
length by default. See the \ref ENC28J60Config section to see how the
%ENC28J60 8kB memory is distributed among TX buffer, RX buffer and this
shared buffer.

This "shared circular buffer" indeed creates a virtual private circular
buffer for each client. To do this, the implementation keeps track of the
head of the circular buffer and all tails. Every time a SharedBuffer writes,
writes to the head. Every time a SharedBuffer releases data it does not
need any more, data is only actually freed if that last release happened to
be the last tail.

Consider the following memory area we want to share for 3 "virtual" circular
buffers A, B and C. In the example, tail pointers A, B, C point each to the
first byte to be read.

\code

+---------------------------------------+
|                                       |
| free                                  |
|                                       |
+---------------------------------------+
^
head
tail=head
\endcode
Now, let's say A writes a fragment of data to the buffer:
\code
+---+-----------------------------------+
|   |                                   |
|A0 |  free                             |
|   |                                   |
+---+-----------------------------------+
^   ^
A   head
tail=A
\endcode
And then B and C write to the buffer
\code
+---+-----+---+---------------------------+
|   |     |   |                           |
|A0 | B0  |C0 |  free                     |
|   |     |   |                           |
+---+-----+---+---------------------------+
^   ^     ^   ^
A   B     C   head
tail=A
\endcode
Then B writes again:
\code
+---+-----+---+---+-----------------------+
|   |     |   |   |                       |
|A0 | B0  |C0 |B1 |  free                 |
|   |     |   |   |                       |
+---+-----+---+---+-----------------------+
^   ^     ^       ^
A   B     C       head
tail=A
\endcode
At this point, lets say B decides to read out (release) the first fragment.
Since this is a circular buffer, only the first fragment (the one pointed
out by tail pointer B) may be released:

\code
+---+-----+---+---+-----------------------+
|   |     |   |   |                       |
|A0 |free |C0 |B1 |  free                 |
|   |     |   |   |                       |
+---+-----+---+---+-----------------------+
^         ^   ^   ^
A         C   B   head
tail=A
\endcode
Fragment B0 is released and B's tail pointer is updated to point to B1

This has fragmented our memory space. The "shared" circular buffer
implementation does not keep track of free fragments that exist between used
chunks of memory. The implementation makes an optimistic assumption that
free chunks will eventually coalesce since the buffer is supposed to be used
to keep data for a short amount of time. This assumption saves a lot of
overhead in keeping track of these fragments and maximizes usage of the
limited memory available. The caveat here is that if a service does not free
a fragment in a timely manner it will eventually block **all** services.
This is mitigated by 1) not running many services simultaneously (at the
end, we're in a microcontroller!) and 2) careful service design: making sure
that services release data frequently or time out and release in case of
errors.


Back to our example, when A releases its only fragment, we end up with this:

\code
+---------+---+---+-----------------------+
|         |   |   |                       |
|  free   |C0 |B1 |  free                 |
|         |   |   |                       |
+---------+---+---+-----------------------+
          ^   ^   ^
          C   B   head
tail=C
\endcode
The global `tail` now points to C and all the free space is consolidated in
one chunk (remember this is a circular buffer and therefore it wraps
around).

You can think of this shared circular buffer as a regular circular buffer in
which the head is shared for all and the tail is the one that is most
behind.

This mechanism is implemented in the SharedBuffer class. Each instance of
SharedBuffer represents an independent buffer (like A, B or C in the
example) in the shared memory area. SharedBuffer also takes care of writing
a small header for each fragment that contains a pointer to the next
fragment, the length of the fragment and a checksum of the data in the
fragment that can be combined to calculate the checksum of the
concatenation.

The class includes a method, fillTxBuffer() that reads out fragments and
concatenates them into the transmit buffer, calculating the resulting
checksum.
*/



#ifndef SHARED_BUFFER_H
#define SHARED_BUFFER_H

#include "List.h"

class SharedBuffer : ListItem
{
	
	static uint16_t head;
	static uint16_t usedSpace;
	static List bufferList;

	uint16_t nextRead;
	uint16_t lastWritten;
	
	static uint16_t append(uint16_t len, const byte* data);
	static uint16_t writeAt(uint16_t index, uint16_t len, const byte* data);
	static uint16_t readAt(uint16_t index, uint16_t len, byte* data );


public:

	uint16_t write(uint16_t len, const byte* data);
	uint16_t release();

	SharedBuffer();
	~SharedBuffer();

	void flush();
	bool isEmpty();
	uint16_t fillTxBuffer(uint16_t dstOffset, uint16_t& checksum, uint16_t count=0xFFFF);



};







#endif