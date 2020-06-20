#ifndef THREAD_LIST_H
#define THREAD_LIST_H

#include "../ds/ds.h"

class THREAD_LIST : public SORTED_LIST
{
public:
	virtual int Compare(const OBJECT* visited_obj, const OBJECT* user_obj) const;
   THREAD* getThreadById(DWORD id);
   long64 cpuTime(bool avg);
   float cpuPercent(bool avg);
   void resetTag(TAG_TYPE type);
   void removeUntaggedThreads();
};

#endif//THREAD_LIST_H