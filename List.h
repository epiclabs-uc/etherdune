// test.h

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

	 List();
	 void add(ListItem* item);
	 void remove(ListItem* item);

	 ListItem* first;

};


#endif

