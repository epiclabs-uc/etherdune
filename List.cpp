// 
// 
// 

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

