#include "stdafx.h"
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"


int THREAD_LIST::Compare(const OBJECT* visited_obj, const OBJECT* user_obj) const
{
	THREAD *d1 = (THREAD*) visited_obj;
	THREAD *d2 = (THREAD*) user_obj;

	if(d1->getId() > d2->getId())
		return 1;
	else if(d1->getId() < d2->getId())
		return -1;
	else 
		return 0;
}

THREAD* THREAD_LIST::getThreadById(DWORD id)
{
   THREAD **thread = 0;
   THREAD key(0);
   key.setId(id);
   if(Find(&key, (OBJECT***)&thread) && *thread)
   {
      return *thread;
   }
   return 0;

}

float THREAD_LIST::cpuPercent(bool avg)
{
   float ret = 0;
   int i;
   for(i=0; i<GetCount(); i++)
   {
      THREAD* t = (THREAD* )GetAt(i);
      ret += t->cpuPercent(avg);
   }
   return ret;
}

long64 THREAD_LIST::cpuTime(bool avg)
{
   long64 cput = 0;
   int i;
   for(i=0; i<GetCount(); i++)
   {
      THREAD* t = (THREAD* )GetAt(i);
      cput += t->cpuTime(avg);
   }
   return cput;
}

void THREAD_LIST::resetTag(TAG_TYPE type)
{
   int i;
   for(i=0; i<GetCount(); i++)
   {
      THREAD *t = (THREAD *)GetAt(i);
      if(t)
      {
         t->resetTag(type);
      }
   }
}

void THREAD_LIST::removeUntaggedThreads()
{
   int i;
   for(i=0; i<GetCount(); i++)
   {
      THREAD *t = (THREAD *)GetAt(i);
      if(t)
      {
         if(!t->isTagged(TAG_TYPE_LIVE_PS))
         {
            Remove(t);
            t->deref();
            i--;
         }
      }
   }
}

