// test.h

#ifndef _LIST_h
#define _LIST_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

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

