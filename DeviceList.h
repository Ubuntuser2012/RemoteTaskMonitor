#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H

#include "../ds/ds.h"

class DEVICE_LIST : public SORTED_LIST
{
public:
	virtual int Compare(const OBJECT* visited_obj, const OBJECT* user_obj) const;
};

#endif//DEVICE_LIST_H