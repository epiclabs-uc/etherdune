// EtherDune State tracking class
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

/** \class Stateful
\brief Maintains a state variable and allows to pull out a state string representation for debugging
\details Complements other classes by adding a state variable and providing a way
to obtain a string representation of the current state when debugging.
This class also allows to keep a timer variable associated with the state in case the
state has to time out.

*/


#ifndef _STATEFUL_H_
#define _STATEFUL_H_

#include <ACross.h>

class Stateful
{
protected:
	uint8_t stateTimer; //!< state timer, in ticks
	uint8_t state; //!< state code

	void setState(uint8_t newState, uint8_t timeout);

public :

	inline uint8_t getState()
	{
		return state;
	}

#if _DEBUG
	/// <summary>
	/// Gets the state string representation, for debugging.
	/// </summary>
	/// <returns>the state string representation as PROGMEM string</returns>
	/// <remarks>This function is not compiled in if _DEBUG is not defined</remarks>
	virtual __FlashStringHelper* getStateString() = 0;
#endif

};




#endif