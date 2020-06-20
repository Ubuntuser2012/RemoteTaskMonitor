#include "stdafx.h"
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"


int PROCESS_LIST::Compare(const OBJECT* visited_obj, const OBJECT* user_obj) const
{
	PROCESS *d1 = (PROCESS*) visited_obj;
	PROCESS *d2 = (PROCESS*) user_obj;

	if(d1->getId() > d2->getId())
		return 1;
	else if(d1->getId() < d2->getId())
		return -1;
	else 
		return 0;
}

PROCESS* PROCESS_LIST::getProcessById(DWORD id) const
{
   PROCESS **process = 0;
   PROCESS key;
   key.setId(id);
   if(Find(&key, (OBJECT***)&process) && *process)
   {
      return *process;
   }
   return 0;

}

PROCESS* PROCESS_LIST::addProcess(EXECOBJ_DATA *data, DEVICE* device)
{
   PROCESS *process = getProcessById(data->pid);
   if(!process)
   {
      process = PROCESS::create(data, device);
      if(process)
      {
         Insert2(process, 0, false);
      }
   }
   if(process)
   {
      process->addData(data);
      process->setTag(TAG_TYPE_LIVE_PS);//mark this process, all unmarked processes will be removed later
   }
   return process;
}

THREAD* PROCESS_LIST::addThread(EXECOBJ_DATA *data)
{
   PROCESS *process = getProcessById(data->pid);
   THREAD *thread = 0;
   if(process)
   {
      thread = process->addThread(data);
      thread->setTag(TAG_TYPE_LIVE_PS);//mark this thread, all unmarked processes will be removed later
   }
   return thread;
}

void PROCESS_LIST::removeUntaggedProcesses()
{
   int i;
   for(i=0; i<GetCount(); i++)
   {
      PROCESS *p = (PROCESS *)GetAt(i);
      if(p)
      {
         p->removeUntaggedThreads();

         if(!p->isTagged(TAG_TYPE_LIVE_PS))
         {
            Remove(p);
            p->deref();
            i--;
         }
      }
   }
}

void PROCESS_LIST::resetTag(TAG_TYPE type)
{
   int i;
   for(i=0; i<GetCount(); i++)
   {
      PROCESS *p = (PROCESS *)GetAt(i);
      if(p)
      {
         p->resetTag(type);
         p->resetTagThreads(type);
      }
   }
}

int PROCESS_LIST::GetThreadCount() const
{
   int i, ret = 0;
   for(i=0; i<GetCount(); i++)
   {
      PROCESS *p = (PROCESS *)GetAt(i);
      if(p)
         ret += p->numThreads();
   }
   return ret;
}
/////////////////////////////////////////////////////////////////////////////////

DISPLAY_LIST::DISPLAY_LIST() 
: SORTED_LIST()
, m_sortBy(SORT_BY_NAME)
, m_showThreads(false) 
{}

void DISPLAY_LIST::Destroy()
{
	for (int i = 0; i < GetCount(); ++i)
	{
		PROCESS *p = (PROCESS *)GetAt(i);
		p->deref();
	}
	SORTED_LIST::Destroy();
}

int DISPLAY_LIST::Insert(PROCESS* p)
{
	int ret = Insert2(p, 0, false);
	int i;
	for(i=0; i<p->numThreads(); i++)
	{
		THREAD* t=p->getThread(i);
		if(t)
		{
			t->refptr();
			if (m_showThreads)
			{
				Insert2(t, 0, false);
			}
		}
	}
   return ret;
}

int DISPLAY_LIST::compareByName(EXECOBJ *d1, EXECOBJ *d2) const
{
   int ret;
   char_t s[2][128];
   PROCESS *p1, *p2;
   THREAD *t1, *t2;

   if(EXECOBJ_PROCESS==d1->getType() && EXECOBJ_PROCESS==d2->getType())
   {
      p1 = dynamic_cast<PROCESS*>(d1);
      p2 = dynamic_cast<PROCESS*>(d2);
      swprintf(s[0], L"%s%i", p1->getName(), p1->getId());
      swprintf(s[1], L"%s%i", p2->getName(), p2->getId());
   }
   else if(EXECOBJ_THREAD==d1->getType() && EXECOBJ_THREAD==d2->getType())
   {
      t1 = dynamic_cast<THREAD*>(d1);
      t2 = dynamic_cast<THREAD*>(d2);
      p1 = t1->getProcess();
      p2 = t2->getProcess();
      swprintf(s[0], L"%s%i%s%i", p1->getName(), p1->getId(), t1->getName(), t1->getId());
      swprintf(s[1], L"%s%i%s%i", p2->getName(), p2->getId(), t2->getName(), t2->getId());
   }
   else if(EXECOBJ_PROCESS==d1->getType() && EXECOBJ_THREAD==d2->getType())
   {
      p1 = dynamic_cast<PROCESS*>(d1);
      t2 = dynamic_cast<THREAD*>(d2);
      p2 = t2->getProcess();
      swprintf(s[0], L"%s%i", p1->getName(), p1->getId());
      swprintf(s[1], L"%s%i%s%i", p2->getName(), p2->getId(), t2->getName(), t2->getId());
   }
   else if(EXECOBJ_THREAD==d1->getType() && EXECOBJ_PROCESS==d2->getType())
   {
      t1 = dynamic_cast<THREAD*>(d1);
      p2 = dynamic_cast<PROCESS*>(d2);
      p1 = t1->getProcess();
      swprintf(s[0], L"%s%i%s%i", p1->getName(), p1->getId(), t1->getName(), t1->getId());
      swprintf(s[1], L"%s%i", p2->getName(), p2->getId());
   }
   ret = _wcsicmp(s[1], s[0]);
   return ret;
}

int DISPLAY_LIST::compareByCPU(EXECOBJ *d1, EXECOBJ *d2, bool avg) const
{
   int ret;
   char_t s[2][128];
   PROCESS *p1, *p2;
   THREAD *t1, *t2;

   if(EXECOBJ_PROCESS==d1->getType() && EXECOBJ_PROCESS==d2->getType())
   {
      p1 = dynamic_cast<PROCESS*>(d1);
      p2 = dynamic_cast<PROCESS*>(d2);
      swprintf(s[0], L"%06i%s%i", (int)(p1->cpuPercent(avg)*100), p1->getName(), p1->getId());
      swprintf(s[1], L"%06i%s%i", (int)(p2->cpuPercent(avg)*100), p2->getName(), p2->getId());
   }
   else if(EXECOBJ_THREAD==d1->getType() && EXECOBJ_THREAD==d2->getType())
   {
      t1 = dynamic_cast<THREAD*>(d1);
      t2 = dynamic_cast<THREAD*>(d2);
      p1 = t1->getProcess();
      p2 = t2->getProcess();
      swprintf(s[0], L"%06i%s%i%s%i", (int)(t1->cpuPercent(avg)*100), p1->getName(), p1->getId(), t1->getName(), t1->getId());
      swprintf(s[1], L"%06i%s%i%s%i", (int)(t2->cpuPercent(avg)*100), p2->getName(), p2->getId(), t2->getName(), t2->getId());
   }
   else if(EXECOBJ_PROCESS==d1->getType() && EXECOBJ_THREAD==d2->getType())
   {
      p1 = dynamic_cast<PROCESS*>(d1);
      t2 = dynamic_cast<THREAD*>(d2);
      p2 = t2->getProcess();
      swprintf(s[0], L"%06i%s%i", (int)(p1->cpuPercent(avg)*100), p1->getName(), p1->getId());
      swprintf(s[1], L"%06i%s%i%s%i", (int)(t2->cpuPercent(avg)*100), p2->getName(), p2->getId(), t2->getName(), t2->getId());
   }
   else if(EXECOBJ_THREAD==d1->getType() && EXECOBJ_PROCESS==d2->getType())
   {
      t1 = dynamic_cast<THREAD*>(d1);
      p2 = dynamic_cast<PROCESS*>(d2);
      p1 = t1->getProcess();
      swprintf(s[0], L"%06i%s%i%s%i", (int)(t1->cpuPercent(avg)*100), p1->getName(), p1->getId(), t1->getName(), t1->getId());
      swprintf(s[1], L"%06i%s%i", (int)(p2->cpuPercent(avg)*100), p2->getName(), p2->getId());
   }
   ret = _wcsicmp(s[1], s[0]);
   return ret;
}

int DISPLAY_LIST::compareByMem(EXECOBJ *d1, EXECOBJ *d2) const
{
   int ret;
   int i[2];
   PROCESS *p1, *p2;
   THREAD *t1, *t2;

   if(EXECOBJ_PROCESS==d1->getType() && EXECOBJ_PROCESS==d2->getType())
   {
      p1 = dynamic_cast<PROCESS*>(d1);
      p2 = dynamic_cast<PROCESS*>(d2);
	  i[0] = p1->memoryUsed();
	  i[1] = p2->memoryUsed();
   }
   else if(EXECOBJ_THREAD==d1->getType() && EXECOBJ_THREAD==d2->getType())
   {
      t1 = dynamic_cast<THREAD*>(d1);
      t2 = dynamic_cast<THREAD*>(d2);
	  i[0] = t1->getProcess()->memoryUsed();
	  i[1] = t2->getProcess()->memoryUsed();
   }
   else if(EXECOBJ_PROCESS==d1->getType() && EXECOBJ_THREAD==d2->getType())
   {
      p1 = dynamic_cast<PROCESS*>(d1);
      t2 = dynamic_cast<THREAD*>(d2);
	  i[0] = p1->memoryUsed();
	  i[1] = t2->getProcess()->memoryUsed();
   }
   else if(EXECOBJ_THREAD==d1->getType() && EXECOBJ_PROCESS==d2->getType())
   {
      t1 = dynamic_cast<THREAD*>(d1);
      p2 = dynamic_cast<PROCESS*>(d2);
	  i[0] = t1->getProcess()->memoryUsed();
	  i[1] = p2->memoryUsed();
   }
   return i[0] > i[1] ? 1 : -1;
}

int DISPLAY_LIST::compareById(EXECOBJ *d1, EXECOBJ *d2) const
{
	int i[2];
	PROCESS *p1, *p2;
	THREAD *t1, *t2;

	if (EXECOBJ_PROCESS == d1->getType() && EXECOBJ_PROCESS == d2->getType())
	{
		p1 = dynamic_cast<PROCESS*>(d1);
		p2 = dynamic_cast<PROCESS*>(d2);
		i[0] = p1->getId();
		i[1] = p2->getId();
	}
	else if (EXECOBJ_THREAD == d1->getType() && EXECOBJ_THREAD == d2->getType())
	{
		t1 = dynamic_cast<THREAD*>(d1);
		t2 = dynamic_cast<THREAD*>(d2);
		i[0] = t1->getId();
		i[1] = t2->getId();
	}
	else if (EXECOBJ_PROCESS == d1->getType() && EXECOBJ_THREAD == d2->getType())
	{
		p1 = dynamic_cast<PROCESS*>(d1);
		t2 = dynamic_cast<THREAD*>(d2);
		i[0] = p1->getId();
		i[1] = t2->getId();
	}
	else if (EXECOBJ_THREAD == d1->getType() && EXECOBJ_PROCESS == d2->getType())
	{
		t1 = dynamic_cast<THREAD*>(d1);
		p2 = dynamic_cast<PROCESS*>(d2);
		p1 = t1->getProcess();
		i[0] = t1->getId();
		i[1] = p2->getId();
	}
	return (i[1] > i[0]) ? 1 : -1;
}

int DISPLAY_LIST::compareByCore(EXECOBJ *d1, EXECOBJ *d2) const
{
	int i[2];
	PROCESS *p1, *p2;
	THREAD *t1, *t2;

	if (EXECOBJ_PROCESS == d1->getType() && EXECOBJ_PROCESS == d2->getType())
	{
		p1 = dynamic_cast<PROCESS*>(d1);
		p2 = dynamic_cast<PROCESS*>(d2);
		i[0] = p1->affinity();
		i[1] = p2->affinity();
	}
	else if (EXECOBJ_THREAD == d1->getType() && EXECOBJ_THREAD == d2->getType())
	{
		t1 = dynamic_cast<THREAD*>(d1);
		t2 = dynamic_cast<THREAD*>(d2);
		i[0] = t1->affinity();
		i[1] = t2->affinity();
	}
	else if (EXECOBJ_PROCESS == d1->getType() && EXECOBJ_THREAD == d2->getType())
	{
		p1 = dynamic_cast<PROCESS*>(d1);
		t2 = dynamic_cast<THREAD*>(d2);
		i[0] = p1->affinity();
		i[1] = t2->affinity();
	}
	else if (EXECOBJ_THREAD == d1->getType() && EXECOBJ_PROCESS == d2->getType())
	{
		t1 = dynamic_cast<THREAD*>(d1);
		p2 = dynamic_cast<PROCESS*>(d2);
		p1 = t1->getProcess();
		i[0] = t1->affinity();
		i[1] = p2->affinity();
	}
	return (i[1] > i[0]) ? 1 : -1;
}

int DISPLAY_LIST::compareByMaxCpu(EXECOBJ *d1, EXECOBJ *d2) const
{
	int ret;
	float i[2];
	PROCESS *p1, *p2;
	THREAD *t1, *t2;

	if (EXECOBJ_PROCESS == d1->getType() && EXECOBJ_PROCESS == d2->getType())
	{
		p1 = dynamic_cast<PROCESS*>(d1);
		p2 = dynamic_cast<PROCESS*>(d2);
		i[0] = p1->getMaxCpu();
		i[1] = p2->getMaxCpu();
	}
	else if (EXECOBJ_THREAD == d1->getType() && EXECOBJ_THREAD == d2->getType())
	{
		t1 = dynamic_cast<THREAD*>(d1);
		t2 = dynamic_cast<THREAD*>(d2);
		i[0] = t1->getProcess()->getMaxCpu();
		i[1] = t2->getProcess()->getMaxCpu();
	}
	else if (EXECOBJ_PROCESS == d1->getType() && EXECOBJ_THREAD == d2->getType())
	{
		p1 = dynamic_cast<PROCESS*>(d1);
		t2 = dynamic_cast<THREAD*>(d2);
		i[0] = p1->getMaxCpu();
		i[1] = t2->getProcess()->getMaxCpu();
	}
	else if (EXECOBJ_THREAD == d1->getType() && EXECOBJ_PROCESS == d2->getType())
	{
		t1 = dynamic_cast<THREAD*>(d1);
		p2 = dynamic_cast<PROCESS*>(d2);
		i[0] = t1->getProcess()->getMaxCpu();
		i[1] = p2->getMaxCpu();
	}
	return (i[1] > i[0]) ? 1 : -1;
}

int DISPLAY_LIST::compareByMaxMem(EXECOBJ *d1, EXECOBJ *d2) const
{
	int i[2];
	PROCESS *p1, *p2;
	THREAD *t1, *t2;

	if (EXECOBJ_PROCESS == d1->getType() && EXECOBJ_PROCESS == d2->getType())
	{
		p1 = dynamic_cast<PROCESS*>(d1);
		p2 = dynamic_cast<PROCESS*>(d2);
		i[0] = p1->getMaxMem();
		i[1] = p2->getMaxMem();
	}
	else if (EXECOBJ_THREAD == d1->getType() && EXECOBJ_THREAD == d2->getType())
	{
		t1 = dynamic_cast<THREAD*>(d1);
		t2 = dynamic_cast<THREAD*>(d2);
		i[0] = t1->getProcess()->getMaxMem();
		i[1] = t2->getProcess()->getMaxMem();
	}
	else if (EXECOBJ_PROCESS == d1->getType() && EXECOBJ_THREAD == d2->getType())
	{
		p1 = dynamic_cast<PROCESS*>(d1);
		t2 = dynamic_cast<THREAD*>(d2);
		i[0] = p1->getMaxMem();
		i[1] = t2->getProcess()->getMaxMem();
	}
	else if (EXECOBJ_THREAD == d1->getType() && EXECOBJ_PROCESS == d2->getType())
	{
		t1 = dynamic_cast<THREAD*>(d1);
		p2 = dynamic_cast<PROCESS*>(d2);
		p1 = t1->getProcess();
		i[0] = t1->getProcess()->getMaxMem();
		i[1] = p2->getMaxMem();
	}
	return (i[1] > i[0]) ? 1 : -1;
}

int DISPLAY_LIST::Compare(const OBJECT* visited_obj, const OBJECT* user_obj) const
{
   EXECOBJ *d1 = (EXECOBJ*) visited_obj;
   EXECOBJ *d2 = (EXECOBJ*) user_obj;
   int ret;

   switch(m_sortBy)
   {
   case SORT_BY_NAME:
      ret = compareByName(d1, d2);
      break;
   case SORT_BY_NAME_R:
      ret = compareByName(d2, d1);
      break;
   case SORT_BY_ID:
	   ret = compareById(d1, d2);
	   break;
   case SORT_BY_ID_R:
	   ret = compareById(d2, d1);
	   break;
   case SORT_BY_CPU:
      ret = compareByCPU(d1, d2, 0);
      break;
   case SORT_BY_CPU_R:
      ret = compareByCPU(d2, d1, 0);
      break;
   case SORT_BY_CORE:
	   ret = compareByCore(d1, d2);
	   break;
   case SORT_BY_CORE_R:
	   ret = compareByCore(d2, d1);
	   break;
   case SORT_BY_CPU_AVG:
      ret = compareByCPU(d1, d2, 1);
      break;
   case SORT_BY_CPU_AVG_R:
      ret = compareByCPU(d2, d1, 1);
      break;
   case SORT_BY_CPU_MAX:
	   ret = compareByMaxCpu(d1, d2);
	   break;
   case SORT_BY_CPU_MAX_R:
	   ret = compareByMaxCpu(d2, d1);
	   break;   
   case SORT_BY_MEM:
	   ret = compareByMem(d1, d2);
	   break;
   case SORT_BY_MEM_R:
	   ret = compareByMem(d2, d1);
	   break;
   case SORT_BY_MEM_MAX:
	   ret = compareByMaxMem(d1, d2);
	   break;
   case SORT_BY_MEM_MAX_R:
	   ret = compareByMaxMem(d2, d1);
	   break;
   }
   return ret;
}

void DISPLAY_LIST::setSortTechnique(SORT_BY sortTechnique)
{
	int sortBy = (int)m_sortBy;
	if (m_sortBy == sortTechnique)
	{
		sortBy |= ((sortBy & 0x1) ^ 0x1);
		m_sortBy = (SORT_BY)sortBy;
	}
	else
	{
		m_sortBy = sortTechnique;
	}
}


int DISPLAY_LIST::getSortTechnique()
{
   return m_sortBy;
}

void DISPLAY_LIST::setShowThreads(bool show)
{
   m_showThreads = show;
}


bool DISPLAY_LIST::getShowThreads()
{
   return m_showThreads;
}

void DISPLAY_LIST::toggleShowThreads()
{
   m_showThreads ^= 1;
}

void DISPLAY_LIST::copyMembers(const DISPLAY_LIST &dl)
{
   m_sortBy=dl.m_sortBy;
   m_showThreads=dl.m_showThreads;
}
