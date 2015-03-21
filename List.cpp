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

#include "List.h"

ListItem::ListItem() :nextItem(NULL)
{
	
}

List::List() : first(NULL)
{
	
}

void List::add(ListItem* item)
{
	item->nextItem = first;
	first = item;
}

void List::remove(ListItem* item)
{
	ListItem* last = NULL;
	for (ListItem* n = first; n != NULL; last = n, n = n->nextItem)
	{
		if (n == item)
		{
			if (last == NULL)
				first = n->nextItem;
			else
				last->nextItem = n->nextItem;

			return;
		}
	}
}

