// EtherDune Linked list class
// Author: Javier Peletier <jm@friendev.com>
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

/// \class List
/// \brief Basic linked list class

/// \class ListItem
/// \brief Base list item class

#ifndef _LIST_h
#define _LIST_h

#include <ACross.h>
#include "config.h"

class List;

class ListItem
{
	friend class List;

public:

	ListItem();

protected:

	ListItem* nextItem;
	
};

class List
{
public:	

	ListItem* first;

	List();
	
	void add(ListItem* item);
	void remove(ListItem* item);

};


#endif

