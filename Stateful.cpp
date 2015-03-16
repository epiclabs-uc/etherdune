// EtherFlow State tracking class
// Author: Javier Peletier <jm@friendev.com>
// Summary: Maintains a state variable and allows to pull out a state string representation for debugging
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

#include "Stateful.h"


#define AC_LOGLEVEL 6
#include <ACLog.h>
ACROSS_MODULE("Stateful");

/// <summary>
/// Changes to a new state
/// </summary>
/// <param name="newState">The new state.</param>
/// <param name="timeout">The timeout, in ticks</param>
void Stateful::setState(uint8_t newState, uint8_t timeout)
{
	state = newState;
	stateTimer = timeout;

	ACDEBUG("set state=%S", getStateString());
}